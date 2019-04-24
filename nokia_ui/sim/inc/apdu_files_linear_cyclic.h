#ifndef _APDU_FILES_LINEAR_CYCLIC_H_
#define _APDU_FILES_LINEAR_CYCLIC_H_

#include "sim.h"
#include "msg.h"

struct msg *apdu_read_record(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le);
struct msg *apdu_update_record(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le);
//struct msg *apdu_seek(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le);

#endif
