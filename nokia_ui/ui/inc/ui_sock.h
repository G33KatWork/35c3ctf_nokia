#ifndef _UI_SOCK_H_
#define _UI_SOCK_H_

#include <osmocom/core/write_queue.h>

struct bb_sock_state {
    void *inst;
    struct osmo_fd conn_bfd;
    struct llist_head upqueue;
};

// Transmit from UI to baseband
int bb_sock_from_ui(struct bb_sock_state *state, struct msgb *msg);
void bb_sock_write_pending(struct bb_sock_state *state);

// Init functions
struct bb_sock_state *bb_sock_init(void* inst, const char *name);
void bb_sock_exit(struct bb_sock_state *state);

#endif
