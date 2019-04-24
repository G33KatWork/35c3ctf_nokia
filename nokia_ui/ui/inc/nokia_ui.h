#ifndef _NOKIA_UI_H_
#define _NOKIA_UI_H_

#include <stdint.h>

#include <osmocom/core/write_queue.h>
#include <osmocom/core/timer.h>

struct screen;
struct nokia_ui;

struct bb_entity {
    int (*bb_msg_cb)(struct nokia_ui *ui, struct msgb *msg);
    struct bb_sock_state *sock_state;
};

struct display_entity {
    void* ui_subsys;
};

struct ui_entity {
    struct osmo_timer_list render_timer;
    struct llist_head event_queue;
    int quit;

    struct screen* screen_current;

    bool on_network;
    int mcc;
    int mnc;

    int unread_sms;
};

struct nokia_ui {
    struct bb_entity bb_entity;
    struct display_entity display_entity;
    struct ui_entity ui_entity;
};

#endif
