#include "sim.h"

#include <stdlib.h>
#include <string.h>

#include "logging.h"
#include "tz/utils.h"
#include "apdu.h"

#define SIM_OBJ_ID  "simcard"

struct simcard *sim_init(void)
{
    struct simcard *sim = NULL;
    char *simdata = NULL;

    sim = calloc(1, sizeof(struct simcard));
    if(!sim)
        return NULL;

    // Read JSON file
    simdata = util_readfile(SIM_OBJ_ID);
    if(!simdata) {
        EMSG("Error opening sim JSON file\n");
        goto out_err;
    }

    sim->json_simdata = cJSON_Parse(simdata);
    if(!sim->json_simdata) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if(error_ptr != NULL)
            EMSG("Error parsing SIM JSON: %s\n", error_ptr);
    }

    free(simdata);

    //FIXME: SIM should select MF implictly. GET_RESPONSE should return results then as well

    return sim;

out_err:
    cJSON_Delete(sim->json_simdata);

    free(sim);
    free(simdata);

    return NULL;
}

void sim_save_to_file(struct simcard *sim)
{
    char *jsonstr = cJSON_Print(sim->json_simdata);
    util_writefile(SIM_OBJ_ID, jsonstr);
    cJSON_free(jsonstr);
}

void sim_exit(struct simcard *sim)
{
    free(sim);
}

void sim_set_pending_reply(struct simcard *sim, struct msg *msg)
{
    if(sim->pending_reply)
        msg_free(sim->pending_reply);

    sim->pending_reply = msg;
}
