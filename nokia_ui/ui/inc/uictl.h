#ifndef _UICTL_H_
#define _UICTL_H_

#include "ui.h"
#include "primitives.h"

struct msgb* uictl_msgb_alloc(void);

void uictl_send_mobile_start(struct nokia_ui *ui);
void uictl_send_mobile_shutdown(struct nokia_ui *ui, uint8_t force);
void uictl_send_tpdu(struct nokia_ui *ui, uint8_t msg_ref, uint8_t tpdu_len, uint8_t *tpdu);
void uictl_send_pin(struct nokia_ui *ui, char *pin1, char *pin2, int cause);

int uictl_bb_sock_to_ui_cb(struct nokia_ui *ui, struct msgb *msg);

const char* prim_to_str(enum mobile_prims p);
const char* prim_op_to_str(enum osmo_prim_operation o);

#endif
