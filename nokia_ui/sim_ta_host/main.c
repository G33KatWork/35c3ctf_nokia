#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <tee_client_api.h>
#include <sim_ta.h>

/* TEE resources */
struct test_ctx {
    TEEC_Context ctx;
    TEEC_Session sess;
};

char *util_readfile(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if(!f)
        return NULL;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *content = malloc(fsize + 1);
    fread(content, fsize, 1, f);
    fclose(f);

    content[fsize] = 0;

    return content;
}

void prepare_tee_session(struct test_ctx *ctx)
{
    TEEC_UUID uuid = TA_SIM_UUID;
    uint32_t origin;
    TEEC_Result res;

    /* Initialize a context connecting us to the TEE */
    res = TEEC_InitializeContext(NULL, &ctx->ctx);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

    /* Open a session with the TA */
    res = TEEC_OpenSession(&ctx->ctx, &ctx->sess, &uuid,
                   TEEC_LOGIN_PUBLIC, NULL, NULL, &origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
            res, origin);
}

void terminate_tee_session(struct test_ctx *ctx)
{
    TEEC_CloseSession(&ctx->sess);
    TEEC_FinalizeContext(&ctx->ctx);
}

TEEC_Result sim_provision(struct test_ctx *ctx, char *data, size_t data_len)
{
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
                     TEEC_NONE, TEEC_NONE, TEEC_NONE);

    op.params[0].tmpref.buffer = data;
    op.params[0].tmpref.size = data_len;

    res = TEEC_InvokeCommand(&ctx->sess,
                 TA_SIM_CMD_PROVISION,
                 &op, &origin);
    if (res != TEEC_SUCCESS)
        printf("Provisioning failed: 0x%x / %u\n", res, origin);

    switch (res) {
    case TEEC_SUCCESS:
        break;
    default:
        printf("Provisioning failed: 0x%x / %u\n", res, origin);
    }

    return res;
}

int main(int argc, char *argv[])
{
    struct test_ctx ctx;
    TEEC_Result res;
    char *file_contents;

    if(argc != 2)
        errx(1, "Usage: %s <sim.json>", argv[0]);

    prepare_tee_session(&ctx);
    file_contents = util_readfile(argv[1]);

    if(!file_contents)
        errx(2, "Failed to load plain text SIM");

    res = sim_provision(&ctx, file_contents, strlen(file_contents));
    if (res != TEEC_SUCCESS)
        errx(3, "Failed to provision SIM card");

    free(file_contents);
    terminate_tee_session(&ctx);
    return 0;
}
