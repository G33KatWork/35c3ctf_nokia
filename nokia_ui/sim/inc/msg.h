#ifndef _MSG_H_
#define _MSG_H_

#include <stdint.h>

#include "linuxlist.h"

struct msg {
    struct llist_head list;
    uint16_t len;
    uint8_t data[0];
};

#define msg_data(m)     ((void*)&(m->data[0]))
#define msg_len(m)      (m->len)

struct msg *msg_alloc(uint16_t size);
void msg_free(struct msg *m);

void msg_enqueue(struct llist_head *queue, struct msg *msg);
struct msg *msg_dequeue(struct llist_head *queue);

#endif
