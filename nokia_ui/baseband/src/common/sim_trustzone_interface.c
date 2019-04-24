#include <osmocom/bb/common/sim_trustzone_interface.h>

#include <osmocom/bb/common/logging.h>
#include <osmocom/bb/mobile/primitives.h>

#include <tee_client_api.h>
#include <sim_ta.h>

#define SOCKET_TZ_MAX_MSG_LEN  (5+256)

struct sim_ctx {
    bool initialized;
    TEEC_Context ctx;
    TEEC_Session sess;
};

struct sim_ctx tz_ctx;

static int trustzonecard_prepare_context(struct sim_ctx *ctx)
{
    TEEC_UUID uuid = TA_SIM_UUID;
    uint32_t origin;
    TEEC_Result res;

    tz_ctx.initialized = false;

    /* Initialize a context connecting us to the TEE */
    res = TEEC_InitializeContext(NULL, &ctx->ctx);
    if(res != TEEC_SUCCESS) {
        LOGP(DSIM, LOGL_ERROR, "TEEC_InitializeContext failed with code 0x%x", res);
        return 1;
    }

    /* Open a session with the TA */
    res = TEEC_OpenSession(&ctx->ctx, &ctx->sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &origin);
    if(res != TEEC_SUCCESS) {
        LOGP(DSIM, LOGL_ERROR, "TEEC_Opensession failed with code 0x%x origin 0x%x", res, origin);
        TEEC_FinalizeContext(&ctx->ctx);
        return 1;
    }

    tz_ctx.initialized = true;

    return 0;
}

int trustzonecard_open(struct osmocom_ms *ms)
{
    struct gsm_subscriber *subscr = &ms->subscr;
    //struct msgb *nmsg;
    int rc;

    if (subscr->sim_valid) {
        LOGP(DMM, LOGL_ERROR, "Cannot insert card, until current card is detached.\n");
        return -EBUSY;
    }

    /* reset subscriber */
    gsm_subscr_exit(ms);
    gsm_subscr_init(ms);

    subscr->sim_type = GSM_SIM_TYPE_TRUSTZONE;
    sprintf(subscr->sim_name, "socket");
    subscr->sim_valid = 1;

    if(trustzonecard_prepare_context(&tz_ctx)) {
        rc = -ENOMEM;
        goto out_err;
    }

    LOGP(DSIM, LOGL_DEBUG, "Trustzone SIM card initialized\n");

    return 0;

out_err:
    /* Detach SIM */
    subscr->sim_valid = 0;
    mobile_prim_ntfy_sim_status(ms, PRIM_SIM_INVALID_MISSING, 0);
    //nmsg = gsm48_mmr_msgb_alloc(GSM48_MMR_NREG_REQ);
    //if (!nmsg)
    //    return -ENOMEM;
    //gsm48_mmr_downmsg(ms, nmsg);

    return rc;
}

int trustzonecard_close(struct osmocom_ms *ms)
{
    if(!tz_ctx.initialized)
        return -EINVAL;

    TEEC_CloseSession(&tz_ctx.sess);
    TEEC_FinalizeContext(&tz_ctx.ctx);

    return 0;
}

int trustzonecard_send_apdu(struct osmocom_ms *ms, uint8_t *data, uint16_t length)
{
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;
    struct msgb *reply_msg;
    uint8_t tmp_reply[SOCKET_TZ_MAX_MSG_LEN] = {0};

    if(!tz_ctx.initialized)
        return -EINVAL;

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
                     TEEC_MEMREF_TEMP_OUTPUT,
                     TEEC_NONE, TEEC_NONE);

    op.params[0].tmpref.buffer = data;
    op.params[0].tmpref.size = length;

    op.params[1].tmpref.buffer = tmp_reply;
    op.params[1].tmpref.size = sizeof(tmp_reply);

    res = TEEC_InvokeCommand(&tz_ctx.sess,
                 TA_SIM_CMD_TRANSFER,
                 &op, &origin);

    if (res != TEEC_SUCCESS) {
        LOGP(DSIM, LOGL_ERROR, "Trustzone SIM APDU transfer failed: 0x%x / %u\n", res, origin);
        goto send_dummy_error;
    }

    reply_msg = msgb_alloc(SOCKET_TZ_MAX_MSG_LEN, "trustzonesim");
    if (!reply_msg) {
        LOGP(DSIM, LOGL_ERROR, "Failed to allocate reply_msg.\n");
        return -1;
    }

    memcpy(msgb_put(reply_msg, op.params[1].tmpref.size), tmp_reply, op.params[1].tmpref.size);
    sim_apdu_resp(ms, reply_msg);

    return 0;

send_dummy_error:
    reply_msg = msgb_alloc(2, "trustzonesim");
    if(!reply_msg)
        return -1;

    msgb_put_u8(reply_msg, 0x6f);
    msgb_put_u8(reply_msg, 0x00);
    sim_apdu_resp(ms, reply_msg);

    return 0;
}
