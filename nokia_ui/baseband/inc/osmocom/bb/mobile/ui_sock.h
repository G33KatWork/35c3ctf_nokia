#ifndef _UI_SOCK_H
#define _UI_SOCK_H

#include <osmocom/core/write_queue.h>

struct ui_sock_state {
    void *inst;
    struct osmo_fd listen_bfd;  /* fd for listen socket */
    struct osmo_fd conn_bfd;        /* fd for connection to lcr */
    struct llist_head upqueue;
};

// Transmit from baseband to UI
int ui_sock_from_baseband(struct ui_sock_state *state, struct msgb *msg);
void ui_sock_write_pending(struct ui_sock_state *state);

// Init functions
struct ui_sock_state *ui_sock_init(void *inst, const char *name);
void ui_sock_exit(struct ui_sock_state *state);

#endif /* _UI_SOCK_H */
