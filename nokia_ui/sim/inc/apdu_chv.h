#ifndef _APDU_CHV_
#define _APDU_CHV_

#include "sim.h"
#include "msg.h"

struct msg *apdu_verify_chv(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le);
struct msg *apdu_change_chv(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le);
struct msg *apdu_disable_chv(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le);
struct msg *apdu_enable_chv(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le);
struct msg *apdu_unblock_chv(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le);

#endif
