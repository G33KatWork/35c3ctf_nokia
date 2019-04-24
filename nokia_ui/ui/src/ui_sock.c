#include "ui_sock.h"

#include <osmocom/core/socket.h>

#include <assert.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#include <nokia_ui.h>

// FIXME: get rid of this? length in init etc.
#include <primitives.h>

// Transmit from UI to baseband
int bb_sock_from_ui(struct bb_sock_state *state, struct msgb *msg)
{
    /* Check if we currently have a UI handler connected */
    if (state->conn_bfd.fd < 0) {
        printf("UI socket seems to be gone\n");
        msgb_free(msg);
        return -1;
    }

    /* prepend 16 bit length */
    //msgb_push_u16(msg, msg->len);

    /* FIXME: check for some maximum queue depth? */

    /* Actually enqueue the message and mark socket write need */
    msgb_enqueue(&state->upqueue, msg);
    state->conn_bfd.when |= BSC_FD_WRITE;
    return 0;
}

void bb_sock_write_pending(struct bb_sock_state *state)
{
    state->conn_bfd.when |= BSC_FD_WRITE;
}

static void bb_sock_close(struct bb_sock_state *state)
{
    struct osmo_fd *bfd = &state->conn_bfd;
    struct nokia_ui *ui = state->inst;

    printf("BB Socket has closed connection\n");

    close(bfd->fd);
    bfd->fd = -1;
    osmo_fd_unregister(bfd);

    /* FIXME: make sure we don't enqueue anymore */

    //tell UI to exit
    ui->ui_entity.quit = 1;

    /* flush the queue */
    while (!llist_empty(&state->upqueue)) {
        struct msgb *msg = msgb_dequeue(&state->upqueue);
        msgb_free(msg);
    }
}

static int bb_sock_read(struct osmo_fd *bfd)
{
    struct bb_sock_state *state = bfd->data;
    struct nokia_ui *ui = state->inst;
    struct msgb *msg;
    int rc;

    msg = msgb_alloc(sizeof(struct mobile_prim), "mobile_prim_bb_to_ui");
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

    if(ui->bb_entity.bb_msg_cb)
        rc = ui->bb_entity.bb_msg_cb(ui, msg);

    // FIXME: check if we can really free the message here
    msgb_free(msg);

    return rc;

close:
    msgb_free(msg);
    bb_sock_close(state);
    return -1;
}

static int bb_sock_write(struct osmo_fd *bfd)
{
    struct bb_sock_state *state = bfd->data;
    int rc;

    while (!llist_empty(&state->upqueue)) {
        struct msgb *msg, *msg2;

        /* peek at the beginning of the queue */
        msg = llist_entry(state->upqueue.next, struct msgb, list);

        bfd->when &= ~BSC_FD_WRITE;

        /* bug hunter 8-): maybe someone forgot msgb_put(...) ? */
        if (!msgb_length(msg)) {
            printf("message with ZERO bytes!\n");
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
    bb_sock_close(state);

    return -1;
}

static int bb_sock_cb(struct osmo_fd *bfd, unsigned int flags)
{
    int rc = 0;

    if (flags & BSC_FD_READ)
        rc = bb_sock_read(bfd);

    if (rc < 0)
        return rc;

    if (flags & BSC_FD_WRITE)
        rc = bb_sock_write(bfd);

    return rc;
}

struct bb_sock_state *bb_sock_init(void* inst, const char *name)
{
    struct bb_sock_state *state;
    struct osmo_fd *bfd;
    int rc;

    state = talloc_zero(inst, struct bb_sock_state);
    if (!state)
        return NULL;

    state->inst = inst;
    INIT_LLIST_HEAD(&state->upqueue);
    state->conn_bfd.fd = -1;

    bfd = &state->conn_bfd;

    rc = osmo_sock_unix_init_ofd(bfd, SOCK_SEQPACKET, 0, name, OSMO_SOCK_F_CONNECT);
    if (rc < 0) {
        printf("Could not connect to unix socket: %s\n", strerror(errno));
        talloc_free(state);
        return NULL;
    }

    bfd->cb = bb_sock_cb;
    bfd->when = BSC_FD_READ;
    bfd->data = state;

    printf("UI Socket created\n");

    return state;
}

void bb_sock_exit(struct bb_sock_state *state)
{
    if (state->conn_bfd.fd > -1)
        bb_sock_close(state);
    talloc_free(state);
}
