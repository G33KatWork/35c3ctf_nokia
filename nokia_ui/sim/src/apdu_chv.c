#include "apdu_chv.h"

#include "logging.h"
#include "chv.h"
#include "apdu.h"
#include "apdu_replies.h"
#include "utils.h"

struct msg *apdu_verify_chv(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le)
{
    if(p[1] != 1 && p[1] != 2)
        goto out_err_p1_p2;

    util_sim_pin_prepare((char*)data);
    DMSG("Verify CHV command for CHV%u: %s\n", p[1], (char*)data);

    int chv = p[1] == 1 ? CHV_1 : CHV_2;

    int enabled = chv_is_enabled(sim, chv);
    if(enabled == CHV_STATUS_FALSE)
        return apdu_reply_success(sim, 0, NULL);
    else if(enabled == CHV_STATUS_INTERNAL_ERROR)
        return apdu_reply_app_error(sim, APDU_STAT_TECH_PROBLEM, 0);

    int verify_result = chv_verify(sim, chv, (char*)data);
    if(verify_result == CHV_STATUS_TRUE)
        return apdu_reply_success(sim, 0, NULL);
    else if(verify_result == CHV_STATUS_FALSE)
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_ACCESS_COND_NOT_FULFILLED);
    else if(verify_result == CHV_STATUS_BLOCKED)
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_CHV_BLOCKED);
    else
        return apdu_reply_app_error(sim, APDU_STAT_TECH_PROBLEM, 0);

    return NULL;

out_err_p1_p2:
    return apdu_reply_app_error(sim, APDU_STAT_INCORR_P1_P2, 0);
}

struct msg *apdu_change_chv(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le)
{
    if(p[1] != 1 && p[1] != 2)
        goto out_err;

    char *old_chv = (char *)&data[0];
    char *new_chv = (char *)&data[8];

    util_sim_pin_prepare(old_chv);
    util_sim_pin_prepare(new_chv);
    DMSG("Change CHV command for CHV%u: %s -> %s\n", p[1], old_chv, new_chv);

    int chv = p[1] == 1 ? CHV_1 : CHV_2;
    int change_result = chv_change(sim, chv, old_chv, new_chv);

    if(change_result == CHV_STATUS_TRUE)
        return apdu_reply_success(sim, 0, NULL);
    else if(change_result == CHV_STATUS_FALSE)
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_ACCESS_COND_NOT_FULFILLED);
    else if(change_result == CHV_STATUS_BLOCKED)
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_CHV_BLOCKED);
    else if(change_result == CHV_STATUS_INVALID_STATE)
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_CHV_STATUS_IN_CONTRADICTION);
    else
        return apdu_reply_app_error(sim, APDU_STAT_TECH_PROBLEM, 0);

    return NULL;

out_err:
    return apdu_reply_app_error(sim, APDU_STAT_INCORR_P1_P2, 0);
}

struct msg *apdu_disable_chv(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le)
{
    util_sim_pin_prepare((char*)data);
    DMSG("Disable CHV command for CHV%u: %s\n", p[1], (char*)data);

    int disable_result = chv_disable(sim, CHV_1, (char*)data);

    if(disable_result == CHV_STATUS_TRUE)
        return apdu_reply_success(sim, 0, NULL);
    else if(disable_result == CHV_STATUS_FALSE)
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_ACCESS_COND_NOT_FULFILLED);
    else if(disable_result == CHV_STATUS_BLOCKED)
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_CHV_BLOCKED);
    else if(disable_result == CHV_STATUS_INVALID_STATE)
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_CHV_STATUS_IN_CONTRADICTION);
    else
        return apdu_reply_app_error(sim, APDU_STAT_TECH_PROBLEM, 0);
}

struct msg *apdu_enable_chv(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le)
{
    util_sim_pin_prepare((char*)data);
    DMSG("Enable CHV command for CHV%u: %s\n", p[1], (char*)data);

    int enable_result = chv_enable(sim, CHV_1, (char*)data);

    if(enable_result == CHV_STATUS_TRUE)
        return apdu_reply_success(sim, 0, NULL);
    else if(enable_result == CHV_STATUS_FALSE)
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_ACCESS_COND_NOT_FULFILLED);
    else if(enable_result == CHV_STATUS_BLOCKED)
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_CHV_BLOCKED);
    else if(enable_result == CHV_STATUS_INVALID_STATE)
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_CHV_STATUS_IN_CONTRADICTION);
    else
        return apdu_reply_app_error(sim, APDU_STAT_TECH_PROBLEM, 0);
}

struct msg *apdu_unblock_chv(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le)
{
    char *unblock_chv = (char *)&data[0];
    char *new_chv = (char *)&data[8];

    if(p[1] != 0 && p[1] != 2)
        goto out_err_p1_p2;

    util_sim_pin_prepare(unblock_chv);
    util_sim_pin_prepare(new_chv);

    DMSG("Unblock CHV command for CHV%u: %s -> %s\n", p[1], unblock_chv, new_chv);

    int unblock_result = chv_unblock(sim, p[1] == 0 ? CHV_1 : CHV_2, unblock_chv, new_chv);

    if(unblock_result == CHV_STATUS_TRUE)
        return apdu_reply_success(sim, 0, NULL);
    else if(unblock_result == CHV_STATUS_FALSE)
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_ACCESS_COND_NOT_FULFILLED);
    else if(unblock_result == CHV_STATUS_BLOCKED)
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_CHV_BLOCKED);
    else if(unblock_result == CHV_STATUS_INVALID_STATE)
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_CHV_STATUS_IN_CONTRADICTION);
    else
        return apdu_reply_app_error(sim, APDU_STAT_TECH_PROBLEM, 0);

    return NULL;

out_err_p1_p2:
    return apdu_reply_app_error(sim, APDU_STAT_INCORR_P1_P2, 0);
}
