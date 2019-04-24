#include "ui_event.h"

extern void *ui_ctx;

struct msgb *ui_event_msgb_alloc(int msg_type)
{
    struct msgb *msg;
    struct ui_event_msg *uie;

    msg = msgb_alloc_headroom(sizeof(*uie), 0, "UI event");
    if (!msg)
        return NULL;

    uie = (struct ui_event_msg *)msgb_put(msg, sizeof(*uie));
    uie->msg_type = msg_type;

    return msg;
}

void ui_event_sendmsg(struct nokia_ui *ui, struct msgb *msg)
{
    msgb_enqueue(&ui->ui_entity.event_queue, msg);
}
