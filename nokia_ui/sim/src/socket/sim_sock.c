#include "socket/sim_sock.h"

#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>

#include "logging.h"
#include "socket/socket.h"
#include "utils.h"

#include <arpa/inet.h>

#define SIM_APDU_MAX_LEN    5+256

static void sim_client_sock_close(struct sim_sock_state *state)
{
    IMSG("SIM client seems to have closed connection\n");
    sock_exit(state->conn_sock);
    state->conn_sock = NULL;

    /* re-enable the generation of ACCEPT for new connections */
    state->listen_sock->when |= SOCK_FD_READ;

    /* flush the queue */
    while(!llist_empty(&state->q_tx)) {
        struct msg *msg = msg_dequeue(&state->q_tx);
        msg_free(msg);
    }
}

static int sim_client_sock_read(struct sim_fd *fd)
{
    int rc;
    struct msg *msg = NULL;
    struct sim_sock_state *state = fd->data;
    uint8_t buf[SIM_APDU_MAX_LEN+2];

    DMSG("SIM received data on sock %d\n", fd->fd);

    // peek to receive len
    uint16_t apdu_len;
    rc = recv(fd->fd, &apdu_len, sizeof(apdu_len), MSG_PEEK);
    if(rc == 0)
        goto close;

    if(rc < 0) {
        if(errno == EAGAIN)
            return 0;
        goto close;
    }

    apdu_len = ntohs(apdu_len);
    if(apdu_len > SIM_APDU_MAX_LEN) {
        EMSG("Indicated length is too big: 0x%04x - ignoring message\n", apdu_len);
        return 0;
    }

    msg = msg_alloc(apdu_len);
    if(!msg)
        return -ENOMEM;

    rc = recv(fd->fd, buf, apdu_len+2, 0);
    if(rc == 0)
        goto close;

    if(rc < 0) {
        if(errno == EAGAIN)
            return 0;
        goto close;
    }

    memcpy(msg_data(msg), buf+2, apdu_len);
    state->sim_recv(state->inst, msg);
    msg_free(msg);

    return rc;

close:
    if(msg) msg_free(msg);
    sim_client_sock_close(state);
    return -1;
}

static int sim_client_sock_write(struct sim_fd *fd)
{
    int rc;
    struct sim_sock_state *state = fd->data;

    while (!llist_empty(&state->q_tx)) {
        struct msg *msg, *msg2;

        /* peek at the beginning of the queue */
        msg = llist_entry(state->q_tx.next, struct msg, list);

        fd->when &= ~SOCK_FD_WRITE;

        /* try to send it over the socket */
        rc = write(fd->fd, msg_data(msg), msg_len(msg));
        if (rc == 0)
            goto close;

        if (rc < 0) {
            if (errno == EAGAIN) {
                fd->when |= SOCK_FD_WRITE;
                break;
            }
            goto close;
        }

        /* _after_ we send it, we can deueue */
        msg2 = msg_dequeue(&state->q_tx);
        assert(msg == msg2);
        msg_free(msg);
    }

    return 0;

close:
    sim_client_sock_close(state);
    return -1;
}

static int sim_client_cb(struct sim_fd *fd, unsigned int flags)
{
    int rc = 0;

    if (flags & SOCK_FD_READ)
        rc = sim_client_sock_read(fd);

    if (rc < 0)
        return rc;

    if (flags & SOCK_FD_WRITE)
        rc = sim_client_sock_write(fd);

    return rc;
}

static int sim_sock_accept_cb(struct sim_fd *fd, unsigned int what)
{
    struct sockaddr_un un_addr;
    socklen_t len;
    int rc;
    struct sim_sock_state *state = fd->data;

    len = sizeof(un_addr);
    rc = accept(fd->fd, (struct sockaddr *) &un_addr, &len);
    if(rc < 0) {
        EMSG("Failed to accept a new connection\n");
        return -1;
    }

    if(state->conn_sock != NULL) {
        EMSG("SIM only connects a single client connection. Dropping new connection\n");
        close(rc);
        return 0;
    }

    /* We already have one UI app connected, this is all we support */
    state->listen_sock->when &= ~SOCK_FD_READ;
    IMSG("Connection accepted, new FD is %d\n", rc);

    struct sim_fd *client = sock_register_fd(rc, sim_client_cb, state);
    if(!client) {
        DMSG("Unable to register new client connection\n");
        return -1;
    }

    state->conn_sock = client;

    return 0;
}

struct sim_sock_state *sim_sock_init(void *inst, const char* sock_name)
{
    struct sim_sock_state *state;

    state = calloc(1, sizeof(struct sim_sock_state));
    if(!state)
        return NULL;

    IMSG("Opening socket\n");

    state->listen_sock = sock_init(sock_name, sim_sock_accept_cb, state, SOCK_TYPE_BIND);
    if(!state->listen_sock) {
        EMSG("Error creating listening socket: %s\n", strerror(errno));
        free(state);
        return NULL;
    }

    state->inst = inst;
    state->conn_sock = NULL;
    INIT_LLIST_HEAD(&state->q_tx);

    return state;
}

void sim_sock_exit(struct sim_sock_state *state)
{
    if(state->conn_sock)
        sim_client_sock_close(state);

    sock_exit(state->listen_sock);

    free(state);
}

int sim_sock_write(struct sim_sock_state *state, struct msg *msg)
{
    if(!state->conn_sock) {
        IMSG("Socket to SIM client seems to be gone\n");
        msg_free(msg);
        return -1;
    }

    struct msg *msg_with_len = msg_alloc(msg_len(msg) + 2);
    uint8_t *payload = msg_data(msg_with_len);
    *((uint16_t*)payload) = msg_len(msg);
    payload += 2;
    memcpy(payload, msg_data(msg), msg_len(msg));
    msg_free(msg);

    msg_enqueue(&state->q_tx, msg_with_len);
    state->conn_sock->when |= SOCK_FD_WRITE;
    return 0;
}
