#ifndef _LOGGING_H
#define _LOGGING_H

#define DEBUG
#include <osmocom/core/logging.h>

enum {
    DRSL,
    DRR,
    DPLMN,
    DCS,
    DNB,
    DMM,
    DSS,
    DSMS,
    DPAG,
    DL1C,
    DSUM,
    DSIM,
    DMOB,
    DUI,
};

extern const struct log_info log_info;

#endif /* _LOGGING_H */
