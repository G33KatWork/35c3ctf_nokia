#include "apdu_misc.h"

#include "logging.h"
#include "apdu.h"
#include "apdu_replies.h"
#include "files.h"
#include "chv.h"
#include "comp128v1.h"
#include "utils.h"

#include <string.h>

struct msg *apdu_sleep(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le)
{
    return apdu_reply_success(sim, 0, NULL);
}

struct msg *apdu_invalidate(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le)
{
    DMSG("Invalidating selected file\n");

    // No file selected?
    if(!sim->selected_file)
        return apdu_reply_app_error(sim, APDU_STAT_TECH_PROBLEM, 0);

    if(!file_access_allowed(sim, sim->selected_file, FILE_ACTION_INVALIDATE)) {
        DMSG("Access rights not sufficient\n");
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_ACCESS_COND_NOT_FULFILLED);
    }

    if(file_set_valid(sim->selected_file, false)) {
        sim_save_to_file(sim);
        return apdu_reply_success(sim, 0, NULL);
    }
    else
        return apdu_reply_app_error(sim, APDU_STAT_TECH_PROBLEM, 0);
}

struct msg *apdu_rehabilitate(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le)
{
    DMSG("Rehabilitating selected file\n");

    // No file selected?
    if(!sim->selected_file)
        return apdu_reply_app_error(sim, APDU_STAT_TECH_PROBLEM, 0);

    if(!file_access_allowed(sim, sim->selected_file, FILE_ACTION_REHABILITATE)) {
        DMSG("Access rights not sufficient\n");
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_ACCESS_COND_NOT_FULFILLED);
    }

    if(file_set_valid(sim->selected_file, true)) {
        sim_save_to_file(sim);
        return apdu_reply_success(sim, 0, NULL);
    }
    else
        return apdu_reply_app_error(sim, APDU_STAT_TECH_PROBLEM, 0);
}

struct msg *apdu_run_gsm_algo(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le)
{
    cJSON *ki_json = cJSON_GetObjectItem(sim->json_simdata, "ki");

    if(!ki_json || !cJSON_IsString(ki_json)) {
        EMSG("SIM doesn't contain Ki node\n");
        return apdu_reply_app_error(sim, APDU_STAT_TECH_PROBLEM, 0);
    }

    if(chv_is_enabled(sim, CHV_1) && !sim->chv_unlocked[CHV_1]) {
        DMSG("Access rights not sufficient\n");
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_ACCESS_COND_NOT_FULFILLED);
    }

    uint8_t ki[16] = {0};
    util_unhexlify(ki, ki_json->valuestring, sizeof(ki));

    uint8_t sres[4], kc[8];
    comp128v1(ki, data, sres, kc);

    struct msg *msg = msg_alloc(12);
    uint8_t *reply = msg_data(msg);
    memcpy(reply, sres, sizeof(sres));
    memcpy(reply+4, kc, sizeof(kc));

    sim_set_pending_reply(sim, msg);
    return apdu_reply_success(sim, msg_len(msg), NULL);
}

struct msg *apdu_get_response(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le)
{
    if(!sim->pending_reply) 
        return apdu_reply_app_error(sim, APDU_STAT_TECH_PROBLEM, 0);
    else {
        if(le > msg_len(sim->pending_reply)) le = msg_len(sim->pending_reply);
        return apdu_reply_success(sim, le, msg_data(sim->pending_reply));
    }
}
