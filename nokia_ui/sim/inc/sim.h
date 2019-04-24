#ifndef _SIM_H_
#define _SIM_H_

#include <stdbool.h>

#include "cJSON.h"
#include "msg.h"

#define SIM_APDU_MAX_LEN    5+256

struct simcard {
    cJSON *json_simdata;

    //internal SIM state
    bool chv_unlocked[2];
    cJSON *selected_file;
    struct msg *pending_reply;
};

struct simcard *sim_init(void);
void sim_save_to_file(struct simcard *sim);
void sim_exit(struct simcard *sim);
void sim_set_pending_reply(struct simcard *sim, struct msg *msg);

#endif
