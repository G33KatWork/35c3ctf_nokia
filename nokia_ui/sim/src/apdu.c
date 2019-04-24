#include "apdu.h"

#include <string.h>
#include <stdbool.h>

#include "logging.h"
#include "apdu_chv.h"
#include "apdu_select.h"
#include "apdu_files_transparent.h"
#include "apdu_files_linear_cyclic.h"
#include "apdu_misc.h"
#include "apdu_replies.h"
#include "utils.h"

enum data_direction {
    CMD_DATA_DIR_SEND,
    CMD_DATA_DIR_RECEIVE
};

struct apdu_command {
    uint8_t inst;
    int valid_p1;
    int valid_p2;
    int valid_len;
    enum data_direction data_direction;
    struct msg* (*handler)(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le);
};

struct apdu_command command_tbl[] = {
    {APDU_GSM_INST_SELECT,              0,  0,     2, CMD_DATA_DIR_SEND,    apdu_select},
    //{APDU_GSM_INST_STATUS,              0,  0,    -1, CMD_DATA_DIR_RECEIVE, apdu_status},

    {APDU_GSM_INST_READ_BINARY,        -1, -1,    -1, CMD_DATA_DIR_RECEIVE, apdu_read_binary},
    {APDU_GSM_INST_UPDATE_BINARY,      -1, -1,    -1, CMD_DATA_DIR_SEND,    apdu_update_binary},

    {APDU_GSM_INST_READ_RECORD,        -1, -1,    -1, CMD_DATA_DIR_RECEIVE, apdu_read_record},
    {APDU_GSM_INST_UPDATE_RECORD,      -1, -1,    -1, CMD_DATA_DIR_SEND,    apdu_update_record},
    //{APDU_GSM_INST_SEEK,                0, -1,    -1, CMD_DATA_DIR_SEND,    apdu_seek},
    //{APDU_GSM_INST_INCREASE,            0,  0,     3, CMD_DATA_DIR_SEND,    NULL},

    {APDU_GSM_INST_VERIFY_CHV,          0, -1,     8, CMD_DATA_DIR_SEND,    apdu_verify_chv},
    {APDU_GSM_INST_CHANGE_CHV,          0, -1,  0x10, CMD_DATA_DIR_SEND,    apdu_change_chv},
    {APDU_GSM_INST_DISABLE_CHV,         0,  1,     8, CMD_DATA_DIR_SEND,    apdu_disable_chv},
    {APDU_GSM_INST_ENABLE_CHV,          0,  1,     8, CMD_DATA_DIR_SEND,    apdu_enable_chv},
    {APDU_GSM_INST_UNBLOCK_CHV,         0, -1,  0x10, CMD_DATA_DIR_SEND,    apdu_unblock_chv},

    {APDU_GSM_INST_INVALIDATE,          0,  0,     0, CMD_DATA_DIR_SEND,    apdu_invalidate},
    {APDU_GSM_INST_REHABLILITATE,       0,  0,     0, CMD_DATA_DIR_SEND,    apdu_rehabilitate},

    {APDU_GSM_INST_RUN_GSM_ALGO,        0,  0,  0x10, CMD_DATA_DIR_SEND,    apdu_run_gsm_algo},
    {APDU_GSM_INST_SLEEP,               0,  0,     0, CMD_DATA_DIR_SEND,    apdu_sleep},

    {APDU_GSM_INST_GET_RESPONSE,        0,  0,    -1, CMD_DATA_DIR_RECEIVE, apdu_get_response},

    //{APDU_GSM_INST_TERMINAL_PROFILE,    0,  0,    -1, CMD_DATA_DIR_SEND,    NULL},
    //{APDU_GSM_INST_ENVELOPE,            0,  0,    -1, CMD_DATA_DIR_SEND,    NULL},
    //{APDU_GSM_INST_FETCH,               0,  0,    -1, CMD_DATA_DIR_RECEIVE, NULL},
    //{APDU_GSM_INST_TERMINAL_RESPONSE,   0,  0,    -1, CMD_DATA_DIR_SEND,    NULL},
};

static const struct value_string sim_inst_names[] = {
    { APDU_GSM_INST_SELECT,             "APDU_GSM_INST_SELECT" },
    { APDU_GSM_INST_STATUS,             "APDU_GSM_INST_STATUS" },
    { APDU_GSM_INST_READ_BINARY,        "APDU_GSM_INST_READ_BINARY" },
    { APDU_GSM_INST_UPDATE_BINARY,      "APDU_GSM_INST_UPDATE_BINARY" },
    { APDU_GSM_INST_READ_RECORD,        "APDU_GSM_INST_READ_RECORD" },
    { APDU_GSM_INST_UPDATE_RECORD,      "APDU_GSM_INST_UPDATE_RECORD" },
    { APDU_GSM_INST_SEEK,               "APDU_GSM_INST_SEEK" },
    { APDU_GSM_INST_INCREASE,           "APDU_GSM_INST_INCREASE" },
    { APDU_GSM_INST_VERIFY_CHV,         "APDU_GSM_INST_VERIFY_CHV" },
    { APDU_GSM_INST_CHANGE_CHV,         "APDU_GSM_INST_CHANGE_CHV" },
    { APDU_GSM_INST_DISABLE_CHV,        "APDU_GSM_INST_DISABLE_CHV" },
    { APDU_GSM_INST_ENABLE_CHV,         "APDU_GSM_INST_ENABLE_CHV" },
    { APDU_GSM_INST_UNBLOCK_CHV,        "APDU_GSM_INST_UNBLOCK_CHV" },
    { APDU_GSM_INST_INVALIDATE,         "APDU_GSM_INST_INVALIDATE" },
    { APDU_GSM_INST_REHABLILITATE,      "APDU_GSM_INST_REHABLILITATE" },
    { APDU_GSM_INST_RUN_GSM_ALGO,       "APDU_GSM_INST_RUN_GSM_ALGO" },
    { APDU_GSM_INST_SLEEP,              "APDU_GSM_INST_SLEEP" },
    { APDU_GSM_INST_GET_RESPONSE,       "APDU_GSM_INST_GET_RESPONSE" },
    { APDU_GSM_INST_TERMINAL_PROFILE,   "APDU_GSM_INST_TERMINAL_PROFILE" },
    { APDU_GSM_INST_ENVELOPE,           "APDU_GSM_INST_ENVELOPE" },
    { APDU_GSM_INST_FETCH,              "APDU_GSM_INST_FETCH" },
    { APDU_GSM_INST_TERMINAL_RESPONSE,  "APDU_GSM_INST_TERMINAL_RESPONSE" },
};

static const char *get_inst_name(int value)
{
    return get_value_string(sim_inst_names, value);
}

struct msg *apdu_process(struct simcard *sim, struct msg *msg)
{
    size_t lc;
    size_t le;
    uint8_t *data;

    struct apdu_command *cmd = NULL;
    struct apdu *apdu = msg_data(msg);

    if(msg_len(msg) < 5) {
        DMSG("Received APDU with length <5. Ignoring\n");
        return apdu_reply_app_error(sim, APDU_STAT_TECH_PROBLEM, 0);
    }

    if(apdu->class != APDU_CLASS_GSM) {
        DMSG("Received APDU with unknown class: 0x%02x\n", apdu->class);
        return apdu_reply_app_error(sim, APDU_STAT_WRONG_CLASS, 0);
    }

    for(int i = 0; i < ARRAY_SIZE(command_tbl); i++) {
        if(command_tbl[i].inst == apdu->inst) {
            cmd = &command_tbl[i];
            break;
        }
    }

    if(!cmd) {
        DMSG("Received APDU with unknown inst: 0x%02x\n", apdu->inst);
        return apdu_reply_app_error(sim, APDU_STAT_UKN_INST, 0);
    }

    DMSG("Processing APDU - Class: 0x%02x - Inst: %s (0x%02x) - P0/1: 0x%02x 0x%02x - P3: 0x%02x\n", apdu->class, get_inst_name(apdu->inst), apdu->inst, apdu->p[0], apdu->p[1], apdu->p[2]);

    if(apdu->inst != APDU_GSM_INST_GET_RESPONSE)
        sim_set_pending_reply(sim, NULL);

    if(cmd->valid_p1 != -1 && cmd->valid_p1 != apdu->p[0]) {
        DMSG("Received APDU 0x%02x - P1 is supposed to be 0x%02x, but received 0x%02x\n", apdu->inst, cmd->valid_p1, apdu->p[0]);
        goto err_invalid_param1_2;
    }

    if(cmd->valid_p2 != -1 && cmd->valid_p2 != apdu->p[1]) {
        DMSG("Received APDU 0x%02x - P2 is supposed to be 0x%02x, but received 0x%02x\n", apdu->inst, cmd->valid_p2, apdu->p[1]);
        goto err_invalid_param1_2;
    }

    if(cmd->valid_len != -1 && cmd->valid_len != apdu->p[2]) {
        DMSG("Received APDU 0x%02x - P3 is supposed to be 0x%02x, but received 0x%02x\n", apdu->inst, cmd->valid_len, apdu->p[2]);
        goto err_invalid_param_3;
    }

    if(cmd->data_direction == CMD_DATA_DIR_SEND) {
        if(msg_len(msg) < (5 + apdu->p[2])) {
            DMSG("APDU is supposed to transport 0x%02x bytes of data, but received APDU is too short: 0x%02x\n", apdu->p[2], msg_len(msg) - 5);
            return apdu_reply_app_error(sim, APDU_STAT_TECH_PROBLEM, 0);
        }

        lc = apdu->p[2];
        data = &apdu->data[0];
        le = 0;
    } else {
        lc = 0;
        data = NULL;
        le = apdu->p[2];
        if(le == 0)
            le = 256;
    }

    if(cmd->handler)
        return cmd->handler(sim, &apdu->p[0], lc, data, le);

    return NULL;

err_invalid_param1_2:
    return apdu_reply_app_error(sim, APDU_STAT_INCORR_P1_P2, 0);

err_invalid_param_3:
    return apdu_reply_app_error(sim, APDU_STAT_INCORR_P3, 0);
}
