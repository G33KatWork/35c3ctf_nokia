#ifndef _SIM_SOCK_H_
#define _SIM_SOCK_H_

#include "socket.h"
#include "msg.h"
#include "linuxlist.h"

struct sim_sock_state {
    void *inst;
    struct sim_fd *listen_sock;
    struct sim_fd *conn_sock;
    struct llist_head q_tx;
    int (*sim_recv)(void* inst, struct msg *msg);
};

// Transmit from SIM to Socket
int sim_sock_write(struct sim_sock_state *state, struct msg *msg);

// Init functions
struct sim_sock_state *sim_sock_init(void *inst, const char* sock_name);
void sim_sock_exit(struct sim_sock_state *state);

#endif
