#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "linuxlist.h"

#define SOCK_FD_READ        0x0001
#define SOCK_FD_WRITE       0x0002
#define SOCK_FD_EXCEPT      0x0004

#define SOCK_TYPE_BIND      1
#define SOCK_TYPE_CONNECT   2

struct sim_fd;

typedef int (*sock_cb)(struct sim_fd *sock, unsigned int what);

struct sim_fd {
    struct llist_head list;
    int fd;
    unsigned int when;
    sock_cb cb;
    void *data;
};

struct sim_fd *sock_init(const char *name, sock_cb cb, void* data, int type);
struct sim_fd *sock_register_fd(int fd, sock_cb cb, void* data);
void sock_unregister_fd(struct sim_fd *state);
void sock_exit(struct sim_fd *state);

int sockets_poll();

#endif
