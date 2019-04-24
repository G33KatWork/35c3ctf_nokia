#ifndef _APDU_SELECT_H_
#define _APDU_SELECT_H_

#include "sim.h"
#include "msg.h"

struct msg *apdu_select(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le);
// struct msg *apdu_status(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le);

#endif
