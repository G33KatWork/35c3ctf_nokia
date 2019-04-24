#include "sim.h"

#include <stdlib.h>
#include <string.h>

#include "logging.h"
#include "socket/sim_sock.h"
#include "socket/utils.h"
#include "utils.h"
#include "apdu.h"

#define SIM_FILENAME        "./sim.json"
#define SIM_SOCKETNAME      "/tmp/simcard"

struct sim_sock_state *sock_state;

static int sim_recv_callback(void *inst, struct msg *msg)
{
    struct simcard *sim = inst;

    DMSG("SIM processing APDU %s\n", util_hexdump(msg_data(msg), msg_len(msg)));

    struct msg *reply = apdu_process(sim, msg);
    if(reply)
        sim_sock_write(sock_state, reply);

    return 0;
}

struct simcard *sim_init(void)
{
    struct simcard *sim = NULL;
    char *simdata = NULL;

    sim = calloc(1, sizeof(struct simcard));
    if(!sim)
        return NULL;

    sock_state = sim_sock_init(sim, SIM_SOCKETNAME);
    sock_state->sim_recv = sim_recv_callback;

    // Read JSON file
    simdata = util_readfile(SIM_FILENAME);
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

    if(sock_state)
        sim_sock_exit(sock_state);

    free(sim);
    free(simdata);

    return NULL;
}

void sim_save_to_file(struct simcard *sim)
{
    char *jsonstr = cJSON_Print(sim->json_simdata);
    util_writefile(SIM_FILENAME, jsonstr);
    cJSON_free(jsonstr);
}

void sim_exit(struct simcard *sim)
{
    sim_sock_exit(sock_state);
    free(sim);
}

void sim_set_pending_reply(struct simcard *sim, struct msg *msg)
{
    if(sim->pending_reply)
        msg_free(sim->pending_reply);

    sim->pending_reply = msg;
}
