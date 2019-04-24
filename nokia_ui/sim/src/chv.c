#include "chv.h"

#include "logging.h"
#include "sim.h"

#include <string.h>

static void chv_int_decrement_remaining(struct simcard *sim, const cJSON *chv);
static int chv_int_get_remaining(const cJSON *chv);
static void chv_int_reset_remaining(struct simcard *sim, const cJSON *chv);
// static void chv_int_set_remaining(struct simcard *sim, const cJSON *chv, int val);
static void chv_int_set(struct simcard *sim, cJSON *chv, char *new);
//static bool chv_int_get_enabled(cJSON *chv);
static void chv_int_set_enabled(struct simcard *sim, cJSON *chv, bool enabled);
static int chv_int_auth_and_get_node(struct simcard *sim, enum CHV chv, cJSON **chv_out, char *chvval);
static int chv_int_get_node(struct simcard *sim, enum CHV chv, cJSON **chv_out);

const char* chv_names[] = {
    [CHV_1] = "CHV1",
    [CHV_2] = "CHV2",
    [CHV_UNBLOCK_1] = "UNBLOCK_CHV1",
    [CHV_UNBLOCK_2] = "UNBLOCK_CHV2",
};

const int unblock_chvs[] = {
    [CHV_1] = CHV_UNBLOCK_1,
    [CHV_2] = CHV_UNBLOCK_2,
};


int chv_is_enabled(struct simcard *sim, enum CHV chv)
{
    cJSON *chv_json = NULL, *chv_enabled_json = NULL;
    int r;

    r = chv_int_get_node(sim, chv, &chv_json);
    if(r != CHV_STATUS_TRUE)
        return r;

    chv_enabled_json = cJSON_GetObjectItem(chv_json, "enabled");
    if(!chv_enabled_json || !cJSON_IsBool(chv_enabled_json)) {
        DMSG("can't find node 'enabled' or not boolean under chv node %s\n", chv_names[chv]);
        return CHV_STATUS_INTERNAL_ERROR;
    }

    return cJSON_IsTrue(chv_enabled_json) ? CHV_STATUS_TRUE : CHV_STATUS_FALSE;
}

int chv_get_remaining(struct simcard *sim, enum CHV chv)
{
    cJSON *chv_json = NULL, *remaining_json = NULL;
    int r;

    r = chv_int_get_node(sim, chv, &chv_json);
    if(r != CHV_STATUS_TRUE)
        return 0;

    remaining_json = cJSON_GetObjectItem(chv_json, "remaining");
    if(!remaining_json || !cJSON_IsNumber(remaining_json)) {
        DMSG("can't find node 'remaining' or not number under chv node %s\n", chv_names[chv]);
        return 0;
    }

    return remaining_json->valueint;
}

int chv_verify(struct simcard *sim, enum CHV chv, char *val)
{
    cJSON *chv_json = NULL;

    if(chv != CHV_1 && chv != CHV_2)
        return CHV_STATUS_INVALID_ARG;

    int r = chv_int_auth_and_get_node(sim, chv, &chv_json, val);

    if(r != CHV_STATUS_TRUE)
        return r;

    chv_int_reset_remaining(sim, chv_json);
    sim->chv_unlocked[chv] = true;
    return CHV_STATUS_TRUE;
}

int chv_change(struct simcard *sim, enum CHV chv, char *old, char *new)
{
    cJSON *chv_json = NULL;

    int enabled = chv_is_enabled(sim, chv);
    if(enabled == CHV_STATUS_FALSE)
        return CHV_STATUS_INVALID_STATE;
    else if(enabled == CHV_STATUS_INTERNAL_ERROR)
        return CHV_STATUS_INTERNAL_ERROR;

    int r = chv_int_auth_and_get_node(sim, chv, &chv_json, old);

    if(r != CHV_STATUS_TRUE)
        return r;

    chv_int_reset_remaining(sim, chv_json);
    chv_int_set(sim, chv_json, new);
    return CHV_STATUS_TRUE;
}

int chv_disable(struct simcard *sim, enum CHV chv, char *val)
{
    cJSON *chv_json = NULL;

    int enabled = chv_is_enabled(sim, chv);
    if(enabled == CHV_STATUS_FALSE)
        return CHV_STATUS_INVALID_STATE;
    else if(enabled == CHV_STATUS_INTERNAL_ERROR)
        return CHV_STATUS_INTERNAL_ERROR;

    int r = chv_int_auth_and_get_node(sim, chv, &chv_json, val);

    if(r != CHV_STATUS_TRUE)
        return r;

    chv_int_reset_remaining(sim, chv_json);
    chv_int_set_enabled(sim, chv_json, false);
    return CHV_STATUS_TRUE;
}

int chv_enable(struct simcard *sim, enum CHV chv, char *val)
{
    cJSON *chv_json = NULL;

    int enabled = chv_is_enabled(sim, chv);
    if(enabled == CHV_STATUS_TRUE)
        return CHV_STATUS_INVALID_STATE;
    else if(enabled == CHV_STATUS_INTERNAL_ERROR)
        return CHV_STATUS_INTERNAL_ERROR;

    int r = chv_int_auth_and_get_node(sim, chv, &chv_json, val);

    if(r != CHV_STATUS_TRUE)
        return r;

    chv_int_reset_remaining(sim, chv_json);
    chv_int_set_enabled(sim, chv_json, true);
    return CHV_STATUS_TRUE;
}

int chv_unblock(struct simcard *sim, enum CHV chv, char *val, char *new)
{
    cJSON *unblock_chv_json = NULL, *chv_json = NULL;
    int r;

    if(chv != CHV_1 && chv != CHV_2)
        return CHV_STATUS_INVALID_ARG;

    r = chv_int_auth_and_get_node(sim, unblock_chvs[chv], &unblock_chv_json, val);
    if(r != CHV_STATUS_TRUE)
        return r;

    r = chv_int_get_node(sim, chv, &chv_json);
    if(r != CHV_STATUS_TRUE)
        return r;

    chv_int_reset_remaining(sim, unblock_chv_json);
    chv_int_reset_remaining(sim, chv_json);
    chv_int_set(sim, chv_json, new);
    sim->chv_unlocked[chv] = true;

    return CHV_STATUS_TRUE;
}




static void chv_int_decrement_remaining(struct simcard *sim, const cJSON *chv)
{
    cJSON *remaining = NULL;
    remaining = cJSON_GetObjectItem(chv, "remaining");

    if(remaining->valueint > 0) {
        cJSON_SetNumberValue(remaining, remaining->valueint-1);
    }

    sim_save_to_file(sim);
}

static void chv_int_reset_remaining(struct simcard *sim, const cJSON *chv)
{
    cJSON *remaining = NULL, *reset_remaining = NULL;
    remaining = cJSON_GetObjectItem(chv, "remaining");
    reset_remaining = cJSON_GetObjectItem(chv, "reset_remaining");

    cJSON_SetNumberValue(remaining, reset_remaining->valueint);

    sim_save_to_file(sim);
}

static int chv_int_get_remaining(const cJSON *chv)
{
    cJSON *remaining_json = cJSON_GetObjectItem(chv, "remaining");
    return remaining_json->valueint;
}

// static void chv_int_set_remaining(struct simcard *sim, const cJSON *chv, int val)
// {
//     cJSON *remaining = cJSON_GetObjectItem(chv, "remaining");
//     cJSON_SetNumberValue(remaining, val);

//     sim_save_to_file(sim);
// }

static void chv_int_set(struct simcard *sim, cJSON *chv, char *new)
{
    cJSON_ReplaceItemInObject(chv, "value", cJSON_CreateString(new));
    sim_save_to_file(sim);
}

// static bool chv_int_get_enabled(cJSON *chv)
// {
//     cJSON *enabled = cJSON_GetObjectItem(chv, "enabled");
//     return cJSON_IsTrue(enabled);
// }

static void chv_int_set_enabled(struct simcard *sim, cJSON *chv, bool enabled)
{
    cJSON_ReplaceItemInObject(chv, "enabled", cJSON_CreateBool(enabled));
    sim_save_to_file(sim);
}

static int chv_int_auth_and_get_node(struct simcard *sim, enum CHV chv, cJSON **chv_out, char *chvval)
{
    cJSON *chvs_json = NULL, *chv_json = NULL;

    chvs_json = cJSON_GetObjectItem(sim->json_simdata, "chv");
    if(!chvs_json) {
        DMSG("chv node missing in JSON\n");
        return CHV_STATUS_INTERNAL_ERROR;
    }

    chv_json = cJSON_GetObjectItem(chvs_json, chv_names[chv]);
    if(!chv_json) {
        DMSG("can't find node %s under chv node\n", chv_names[chv]);
        return CHV_STATUS_INTERNAL_ERROR;
    }

    *chv_out = chv_json;

    if(chv_int_get_remaining(chv_json) == 0) {
        DMSG("PIN is blocked\n");
        return CHV_STATUS_BLOCKED;
    }

    char *pin = cJSON_GetStringValue(cJSON_GetObjectItem(chv_json, "value"));

    //35C3 BUG!
    //They told me I could be anything, so I became an Intel ME
    //if(strncmp(pin chvval, 8))
    if(strncmp(pin, chvval, strlen(chvval))) {
        DMSG("PIN is wrong\n");
        chv_int_decrement_remaining(sim, chv_json);

        if(chv_int_get_remaining(chv_json) == 0)
            return CHV_STATUS_BLOCKED;
        else
            return CHV_STATUS_FALSE;
    } else {
        DMSG("PIN is correct\n");
        chv_int_reset_remaining(sim, chv_json);
        return CHV_STATUS_TRUE;
    }
}

static int chv_int_get_node(struct simcard *sim, enum CHV chv, cJSON **chv_out)
{
    cJSON *chvs_json = NULL, *chv_json = NULL;

    chvs_json = cJSON_GetObjectItem(sim->json_simdata, "chv");
    if(!chvs_json) {
        DMSG("chv node missing in JSON\n");
        return CHV_STATUS_INTERNAL_ERROR;
    }

    chv_json = cJSON_GetObjectItem(chvs_json, chv_names[chv]);
    if(!chv_json) {
        DMSG("can't find node %s under chv node\n", chv_names[chv]);
        return CHV_STATUS_INTERNAL_ERROR;
    }

    *chv_out = chv_json;
    return CHV_STATUS_TRUE;
}
