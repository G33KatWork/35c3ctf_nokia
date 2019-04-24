#ifndef _SMS_RECEIVE_H_
#define _SMS_RECEIVE_H_

#include <stdint.h>

#include "nokia_ui.h"

void sms_receive(struct nokia_ui *ui, uint8_t tpdu_len, uint8_t *tpdu);

#endif
