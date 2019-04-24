#ifndef _APDU_FILES_TRANSPARENT_H_
#define _APDU_FILES_TRANSPARENT_H_

#include "sim.h"
#include "msg.h"

struct msg *apdu_read_binary(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le);
struct msg *apdu_update_binary(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le);

#endif
