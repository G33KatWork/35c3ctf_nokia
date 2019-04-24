#include "apdu_select.h"

#include "logging.h"
#include "apdu.h"
#include "files.h"
#include "utils.h"
#include "chv.h"
#include "apdu_replies.h"

static struct msg *apdu_select_status_prepare_reply_mf_df(struct simcard *sim)
{
    cJSON *selected_file = sim->selected_file;
    enum FILE_TYPE filetype = file_get_type(selected_file);

    struct msg *msg = msg_alloc(sizeof(struct apdu_reply_select_mf_df));
    struct apdu_reply_select_mf_df *reply = msg_data(msg);

    enum CHV_STATUS chv1_enabled = chv_is_enabled(sim, CHV_1);
    if(chv1_enabled == CHV_STATUS_INTERNAL_ERROR)
        goto err_internal_err;

    reply->free_mem = 0xFFFF;
    reply->file_id = htons(file_get_id(selected_file));

    if(filetype == -1)
        goto err_internal_err;

    reply->file_type = filetype;

    reply->len_additional_data = 10;
    reply->file_characteristics = chv1_enabled == CHV_STATUS_TRUE ? 0x80 : 0x00;
    reply->num_df = file_get_num_df(selected_file);
    reply->num_ef = file_get_num_ef(selected_file);
    reply->num_codes = 4;

    reply->chv1_status = 0x80 | chv_get_remaining(sim, CHV_1);
    reply->unblock_chv1_status = 0x80 | chv_get_remaining(sim, CHV_UNBLOCK_1);
    reply->chv2_status = 0x80 | chv_get_remaining(sim, CHV_2);
    reply->unblock_chv2_status = 0x80 | chv_get_remaining(sim, CHV_UNBLOCK_2);

    return msg;

err_internal_err:
    if(msg)
        msg_free(msg);

    return NULL;
}

static struct msg * apdu_select_status_prepare_reply_ef(struct simcard *sim)
{
    cJSON *selected_file = sim->selected_file;

    struct msg *msg = msg_alloc(sizeof(struct apdu_reply_select_ef));
    struct apdu_reply_select_ef *reply = msg_data(msg);

    reply->file_size = htons(file_get_size(selected_file));
    reply->file_id = htons(file_get_id(selected_file));

    reply->file_type = FILE_TYPE_EF;

    //FIXME: cyclic files
    reply->cyclic_increase_allowed = 0;

    reply->access_conditions[0] = (file_get_access_condition(selected_file, FILE_ACTION_READ) << 4) | file_get_access_condition(selected_file, FILE_ACTION_UPDATE);
    reply->access_conditions[1] = (file_get_access_condition(selected_file, FILE_ACTION_INCREASE) << 4) | 0;
    reply->access_conditions[2] = (file_get_access_condition(selected_file, FILE_ACTION_REHABILITATE) << 4) | file_get_access_condition(selected_file, FILE_ACTION_INVALIDATE);

    reply->file_status = (file_valid(selected_file) ? 1 : 0) | (file_invalid_allow_rw(selected_file) ? 4 : 0);

    reply->len_additional_data = 2;

    reply->ef_structure = file_get_structure(selected_file);

    //FIXME: cyclic files
    //if(filetype == FILE_TYPE_EF_CYCLIC)
    //    reply->record_len = 0;

    return msg;

//err_internal_err:
//    if(msg)
//        msg_free(msg);

    return NULL;
}

struct msg *apdu_select(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le)
{
    uint16_t file_id = (data[0] << 8) | data[1];
    DMSG("Selecting file 0x%04x\n", file_id);

    cJSON *selected_file = file_find(sim, file_id);

    if(!selected_file) {
        DMSG("didn't find file with ID 0x%04x\n", file_id);
        sim->selected_file = NULL;
        goto err_not_found;
    }

    DMSG("found file with ID 0x%04x\n", file_id);
    sim->selected_file = selected_file;

    struct msg *reply = NULL;

    enum FILE_TYPE filetype = file_get_type(selected_file);
    if(filetype == FILE_TYPE_MF || filetype == FILE_TYPE_DF)
        reply = apdu_select_status_prepare_reply_mf_df(sim);
    else if(filetype == FILE_TYPE_EF)
        reply = apdu_select_status_prepare_reply_ef(sim);
    else
        goto err_internal_err;

    if(reply) {
        DMSG("Selected file with ID 0x%04x\n", file_id);
        sim_set_pending_reply(sim, reply);
        return apdu_reply_success(sim, msg_len(reply), NULL);
    } else {
        DMSG("Can't select file with ID 0x%04x\n", file_id);
        return apdu_reply_mem_error(sim);
    }

    return NULL;

err_not_found:
    DMSG("File ID 0x%04x not found\n", file_id);
    return apdu_reply_ref_error(sim, APDU_REF_ERR_NOT_FOUND);

err_internal_err:
    DMSG("Internal error while selecting file with ID 0x%04x\n", file_id);
    return apdu_reply_mem_error(sim);
}

// void apdu_status(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le)
// {
//     return;
// }
