#ifndef _APDU_REPLIES_H_
#define _APDU_REPLIES_H_

#include "sim.h"
#include "msg.h"

struct msg *apdu_reply_success(struct simcard *sim, size_t datalen, uint8_t *data);
struct msg *apdu_reply_mem_error(struct simcard *sim);
struct msg *apdu_reply_ref_error(struct simcard *sim, uint8_t reason);
struct msg *apdu_reply_app_error(struct simcard *sim, uint8_t sw1, uint8_t sw2);

#endif
