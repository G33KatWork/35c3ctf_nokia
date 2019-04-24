#include "socket/socket.h"

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>

#include <logging.h>
#include "utils.h"

static int maxfd = 0;
static LLIST_HEAD(sim_fds);
static int unregistered_count;

static int sock_create_unix(const char* socket_path, int type)
{
    struct sockaddr_un local;
    unsigned int namelen;
    int sfd, rc;

    local.sun_family = AF_UNIX;

    if(util_strlcpy(local.sun_path, socket_path, sizeof(local.sun_path)) >= sizeof(local.sun_path)) {
        EMSG("Socket path exceeds maximum length of %zd bytes: %s\n", sizeof(local.sun_path), socket_path);
        return -ENOSPC;
    }

    namelen = SUN_LEN(&local);

    sfd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if(sfd < 0)
        return -1;

    if(type == SOCK_TYPE_BIND) {
        unlink(local.sun_path);
        rc = bind(sfd, (struct sockaddr *)&local, namelen);
        if (rc < 0)
            goto err;

        rc = listen(sfd, 10);
        if (rc < 0)
            goto err;
    } else {
        rc = connect(sfd, (struct sockaddr *)&local, namelen);
        if (rc < 0)
            goto err;
    }

    return sfd;

err:
    close(sfd);
    return -1;
}

struct sim_fd *sock_init(const char *name, sock_cb cb, void* data, int type)
{
    struct sim_fd *state;
    int fd;

    fd = sock_create_unix(name, type);
    if(fd < 0) {
        EMSG("Could not listen on unix socket: %s\n", strerror(errno));
        return NULL;
    }

    state = sock_register_fd(fd, cb, data);
    if(!state) {
        return NULL;
    }

    return state;
}

struct sim_fd *sock_register_fd(int fd, sock_cb cb, void* data)
{
    int flags;
    struct sim_fd *state;

    state = calloc(1, sizeof(struct sim_fd));
    if(!state)
        return NULL;

    /* make FD nonblocking */
    flags = fcntl(fd, F_GETFL);
    if(flags < 0)
        goto out_err;
    flags |= O_NONBLOCK;
    flags = fcntl(fd, F_SETFL, flags);
    if(flags < 0)
        goto out_err;

    state->fd = fd;
    state->when = SOCK_FD_READ;
    state->cb = cb;
    state->data = data;

    if(state->fd > maxfd)
        maxfd = state->fd;

    llist_add_tail(&state->list, &sim_fds);

    return state;

out_err:
    free(state);
    return NULL;
}

void sock_exit(struct sim_fd *state)
{
    if(state->fd > -1)
        close(state->fd);

    unregistered_count++;
    llist_del(&state->list);

    free(state);
}

int sockets_poll()
{
    fd_set readset, writeset, exceptset;
    struct sim_fd *ufd, *tmp;
    int rc;

    FD_ZERO(&readset);
    FD_ZERO(&writeset);
    FD_ZERO(&exceptset);

    llist_for_each_entry(ufd, &sim_fds, list) {
        if(ufd->when & SOCK_FD_READ)
            FD_SET(ufd->fd, &readset);
        if(ufd->when & SOCK_FD_WRITE)
            FD_SET(ufd->fd, &writeset);
        if(ufd->when & SOCK_FD_EXCEPT)
            FD_SET(ufd->fd, &exceptset);
    }

    rc = select(maxfd+1, &readset, &writeset, &exceptset, NULL);
    if(rc < 0)
        return 0;

restart:
    unregistered_count = 0;
    llist_for_each_entry_safe(ufd, tmp, &sim_fds, list) {
        int flags = 0;

        if(FD_ISSET(ufd->fd, &readset)) {
            flags |= SOCK_FD_READ;
            FD_CLR(ufd->fd, &readset);
        }

        if(FD_ISSET(ufd->fd, &writeset)) {
            flags |= SOCK_FD_WRITE;
            FD_CLR(ufd->fd, &writeset);
        }

        if(FD_ISSET(ufd->fd, &exceptset)) {
            flags |= SOCK_FD_EXCEPT;
            FD_CLR(ufd->fd, &exceptset);
        }

        if(flags)
            ufd->cb(ufd, flags);

        /* ugly, ugly hack. If more than one filedescriptor was
         * unregistered, they might have been consecutive and
         * llist_for_each_entry_safe() is no longer safe */
        /* this seems to happen with the last element of the list as well */
        if (unregistered_count >= 1)
            goto restart;
    }

    return 0;
}
