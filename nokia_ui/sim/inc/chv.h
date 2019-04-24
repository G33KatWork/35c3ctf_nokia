#ifndef _CHV_H_
#define _CHV_H_

#include "sim.h"

#include <stdint.h>
#include <stdbool.h>

enum CHV {
    CHV_1 = 0,
    CHV_2 = 1,
    CHV_UNBLOCK_1 = 2,
    CHV_UNBLOCK_2 = 3,
};

enum CHV_STATUS {
    CHV_STATUS_FALSE,
    CHV_STATUS_TRUE,
    CHV_STATUS_BLOCKED,
    CHV_STATUS_INVALID_STATE,
    CHV_STATUS_INVALID_ARG,
    CHV_STATUS_INTERNAL_ERROR,
};

int chv_is_enabled(struct simcard *sim, enum CHV chv);
int chv_get_remaining(struct simcard *sim, enum CHV chv);

int chv_verify(struct simcard *sim, enum CHV chv, char* val);
int chv_change(struct simcard *sim, enum CHV chv, char *old, char *new);
int chv_disable(struct simcard *sim, enum CHV chv, char *val);
int chv_enable(struct simcard *sim, enum CHV chv, char *val);
int chv_unblock(struct simcard *sim, enum CHV chv, char *val, char *new);

#endif
