#include "msg.h"

#include <stdlib.h>

struct msg *msg_alloc(uint16_t size)
{
    struct msg *m = calloc(1, sizeof(struct msg) + size);
    m->len = size;

    return m;
}

void msg_free(struct msg *m)
{
    free(m);
}

void msg_enqueue(struct llist_head *queue, struct msg *msg)
{
    llist_add_tail(&msg->list, queue);
}

struct msg *msg_dequeue(struct llist_head *queue)
{
    struct llist_head *lh;

    if(llist_empty(queue))
        return NULL;

    lh = queue->next;

    if(lh) {
        llist_del(lh);
        return llist_entry(lh, struct msg, list);
    } else
        return NULL;
}
