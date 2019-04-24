#include <osmocom/bb/mobile/ui_sock.h>

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <osmocom/core/socket.h>
#include <osmocom/bb/common/osmocom_data.h>
#include <osmocom/bb/common/logging.h>
#include <osmocom/bb/mobile/app_mobile.h>

//FIXME: get rid of this? save packet length in struct, pass it to _init
#include <osmocom/bb/mobile/primitives.h>

// Transmit from baseband to UI
int ui_sock_from_baseband(struct ui_sock_state *state, struct msgb *msg)
{
    /* Check if we currently have a UI handler connected */
    if (state->conn_bfd.fd < 0) {
        LOGP(DUI, LOGL_ERROR, "UI socket seems to be gone\n");
        msgb_free(msg);
        return -1;
    }

    /* Actually enqueue the message and mark socket write need */
    msgb_enqueue(&state->upqueue, msg);
    state->conn_bfd.when |= BSC_FD_WRITE;
    return 0;
}

void ui_sock_write_pending(struct ui_sock_state *state)
{
    state->conn_bfd.when |= BSC_FD_WRITE;
}

static void ui_sock_close(struct ui_sock_state *state)
{
    struct osmo_fd *bfd = &state->conn_bfd;
    struct osmocom_ms *ms = (struct osmocom_ms *)state->inst;

    LOGP(DUI, LOGL_NOTICE, "UI Socket has closed connection\n");

    close(bfd->fd);
    bfd->fd = -1;
    osmo_fd_unregister(bfd);

    /* re-enable the generation of ACCEPT for new connections */
    state->listen_bfd.when |= BSC_FD_READ;

    /* force mobile shutdown in case of UI crash */
    mobile_stop(ms, 1);

    /* flush the queue */
    while (!llist_empty(&state->upqueue)) {
        struct msgb *msg = msgb_dequeue(&state->upqueue);
        msgb_free(msg);
    }
}

static int ui_sock_read(struct osmo_fd *bfd)
{
    struct ui_sock_state *state = bfd->data;
    struct osmocom_ms *ms = (struct osmocom_ms *)state->inst;
    struct msgb *msg;
    int rc;

    msg = msgb_alloc(sizeof(struct mobile_prim), "mobile_prim_ui_to_bb");
    if (!msg)
        return -ENOMEM;

    rc = recv(bfd->fd, msg->tail, msgb_tailroom(msg), 0);
    if (rc == 0)
        goto close;

    if (rc < 0) {
        if (errno == EAGAIN)
            return 0;
        goto close;
    }

    if(ms->ui_entity.ui_recv)
        rc = ms->ui_entity.ui_recv(ms, msg);

    msgb_free(msg);

    return rc;

close:
    msgb_free(msg);
    ui_sock_close(state);
    return -1;
}

static int ui_sock_write(struct osmo_fd *bfd)
{
    struct ui_sock_state *state = bfd->data;
    int rc;

    while (!llist_empty(&state->upqueue)) {
        struct msgb *msg, *msg2;

        /* peek at the beginning of the queue */
        msg = llist_entry(state->upqueue.next, struct msgb, list);

        bfd->when &= ~BSC_FD_WRITE;

        /* bug hunter 8-): maybe someone forgot msgb_put(...) ? */
        if (!msgb_length(msg)) {
            LOGP(DUI, LOGL_ERROR, "message with ZERO bytes!\n");
            goto dontsend;
        }

        /* try to send it over the socket */
        rc = write(bfd->fd, msgb_data(msg), msgb_length(msg));
        if (rc == 0)
            goto close;
        if (rc < 0) {
            if (errno == EAGAIN) {
                bfd->when |= BSC_FD_WRITE;
                break;
            }
            goto close;
        }

dontsend:
        /* _after_ we send it, we can deueue */
        msg2 = msgb_dequeue(&state->upqueue);
        assert(msg == msg2);
        msgb_free(msg);
    }
    return 0;

close:
    ui_sock_close(state);

    return -1;
}

static int ui_sock_cb(struct osmo_fd *bfd, unsigned int flags)
{
    int rc = 0;

    if (flags & BSC_FD_READ)
        rc = ui_sock_read(bfd);
    if (rc < 0)
        return rc;

    if (flags & BSC_FD_WRITE)
        rc = ui_sock_write(bfd);

    return rc;
}

/* accept a new connection */
static int ui_sock_accept(struct osmo_fd *bfd, unsigned int flags)
{
    struct ui_sock_state *state = (struct ui_sock_state *)bfd->data;
    struct osmo_fd *conn_bfd = &state->conn_bfd;
    struct sockaddr_un un_addr;
    socklen_t len;
    int rc;

    len = sizeof(un_addr);
    rc = accept(bfd->fd, (struct sockaddr *) &un_addr, &len);
    if (rc < 0) {
        LOGP(DUI, LOGL_ERROR, "Failed to accept a new connection\n");
        return -1;
    }

    if (conn_bfd->fd >= 0) {
        LOGP(DUI, LOGL_NOTICE, "UI app connects but we already have "
            "another active connection ?!?\n");
        /* We already have one UI app connected, this is all we support */
        state->listen_bfd.when &= ~BSC_FD_READ;
        close(rc);
        return 0;
    }

    conn_bfd->fd = rc;
    conn_bfd->when = BSC_FD_READ;
    conn_bfd->cb = ui_sock_cb;
    conn_bfd->data = state;

    if (osmo_fd_register(conn_bfd) != 0) {
        LOGP(DUI, LOGL_ERROR, "Failed to register new connection fd\n");
        close(conn_bfd->fd);
        conn_bfd->fd = -1;
        return -1;
    }

    LOGP(DUI, LOGL_NOTICE, "UI Socket has connection with external "
        "UI application\n");

    return 0;
}

struct ui_sock_state *ui_sock_init(void *inst, const char *name)
{
    struct ui_sock_state *state;
    struct osmo_fd *bfd;
    int rc;

    state = talloc_zero(inst, struct ui_sock_state);
    if (!state)
        return NULL;

    state->inst = inst;
    INIT_LLIST_HEAD(&state->upqueue);
    state->conn_bfd.fd = -1;

    bfd = &state->listen_bfd;

    rc = osmo_sock_unix_init_ofd(bfd, SOCK_SEQPACKET, 0, name, OSMO_SOCK_F_BIND);
    if (rc < 0) {
        LOGP(DUI, LOGL_ERROR, "Could not create unix socket: %s\n",
            strerror(errno));
        talloc_free(state);
        return NULL;
    }

    bfd->cb = ui_sock_accept;
    bfd->data = state;

    LOGP(DUI, LOGL_NOTICE, "UI Socket created\n");

    return state;
}

void ui_sock_exit(struct ui_sock_state *state)
{
    if (state->conn_bfd.fd > -1)
        ui_sock_close(state);
    osmo_fd_unregister(&state->listen_bfd);
    close(state->listen_bfd.fd);
    talloc_free(state);
}
