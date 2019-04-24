#ifndef _APDU_MISC_H_
#define _APDU_MISC_H_

#include "sim.h"
#include "msg.h"

struct msg *apdu_sleep(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le);

struct msg *apdu_invalidate(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le);
struct msg *apdu_rehabilitate(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le);

struct msg *apdu_run_gsm_algo(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le);

struct msg *apdu_get_response(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le);

#endif
