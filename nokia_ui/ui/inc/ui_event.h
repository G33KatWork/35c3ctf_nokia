#ifndef _UI_EVENT_H_
#define _UI_EVENT_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <osmocom/core/msgb.h>

#include "nokia_ui.h"

#define UI_EVENT_NETWORK_SEARCH_SUCCESS     1
#define UI_EVENT_SMS_SEND_DONE              2
#define UI_EVENT_SIM                        3

struct ui_event_send_sms_done_params {
    uint8_t msg_ref;
    int cause;
};

struct ui_event_sim_params {
    int cause;
    int tries_left;
};

struct ui_event_msg {
    int             msg_type;
    union {
        struct ui_event_send_sms_done_params sms_done_params;
        struct ui_event_sim_params sim_params;
    } u;
};

struct msgb *ui_event_msgb_alloc(int msg_type);
void ui_event_sendmsg(struct nokia_ui *ui, struct msgb *msg);

#endif
