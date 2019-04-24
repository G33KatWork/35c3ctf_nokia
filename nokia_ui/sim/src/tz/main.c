#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include "tz/sim_ta.h"

#include "sim.h"
#include "apdu.h"
#include "tz/utils.h"

#include <string.h>

#define SIM_APDU_MAX_LEN    5+256
#define SIM_OBJ_ID          "simcard"

struct simcard *sim = NULL;

static TEE_Result ta_cmd_sim_provision(uint32_t param_types, TEE_Param params[4]);
static TEE_Result ta_cmd_sim_transfer_apdu(uint32_t param_types, TEE_Param params[4]);

TEE_Result TA_CreateEntryPoint(void)
{
    sim = sim_init();

    IMSG("GSM 11.11 SIM TA loaded");
    return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void)
{
    if(sim) sim_exit(sim);
    IMSG("GSM 11.11 SIM TA unloaded");
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t __unused param_types,
        TEE_Param __unused params[4],
        void __unused **sess_ctx)
{
    return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void __unused *sess_ctx)
{
}

TEE_Result TA_InvokeCommandEntryPoint(void __unused *sess_ctx,
            uint32_t cmd_id,
            uint32_t param_types,
            TEE_Param params[4])
{
    (void)&sess_ctx; /* Unused parameter */

    switch (cmd_id) {
        case TA_SIM_CMD_PROVISION:
            return ta_cmd_sim_provision(param_types, params);
        case TA_SIM_CMD_TRANSFER:
            return ta_cmd_sim_transfer_apdu(param_types, params);
        default:
            return TEE_ERROR_BAD_PARAMETERS;
    }
}

static TEE_Result ta_cmd_sim_provision(uint32_t param_types, TEE_Param params[4])
{
    const uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
                TEE_PARAM_TYPE_NONE,
                TEE_PARAM_TYPE_NONE,
                TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    util_writefile(SIM_OBJ_ID, params[0].memref.buffer);

    return TEE_SUCCESS;
}

static TEE_Result ta_cmd_sim_transfer_apdu(uint32_t param_types, TEE_Param params[4])
{
    int res;
    const uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
                TEE_PARAM_TYPE_MEMREF_OUTPUT,
                TEE_PARAM_TYPE_NONE,
                TEE_PARAM_TYPE_NONE);

    if(!sim) {
        EMSG("Trying to perform transfer without initialized SIM. Returning error");
        return TEE_ERROR_GENERIC;
    }

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    if (params[0].memref.size > SIM_APDU_MAX_LEN)
        return TEE_ERROR_BAD_PARAMETERS;

    struct msg *msg = msg_alloc(params[0].memref.size);
    memcpy(msg_data(msg), params[0].memref.buffer, msg_len(msg));

    struct msg *reply = apdu_process(sim, msg);
    if(msg_len(reply) > params[1].memref.size) {
        EMSG("supplied buffer too small: len reply: %u - passed buffer size: %u\n", msg_len(reply), params[1].memref.size);
        params[1].memref.size = msg_len(reply);
        res = TEE_ERROR_SHORT_BUFFER;
        goto exit;
    }

    memcpy(params[1].memref.buffer, msg_data(reply), msg_len(reply));
    params[1].memref.size = msg_len(reply);
    res = TEE_SUCCESS;

exit:
    msg_free(msg);
    if(reply) msg_free(reply);

    return res;
}
