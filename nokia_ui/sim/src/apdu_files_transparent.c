#include "apdu_files_transparent.h"

#include "logging.h"
#include "apdu.h"
#include "apdu_replies.h"
#include "files.h"
#include "utils.h"

struct msg *apdu_read_binary(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le)
{
    uint16_t file_offset = (p[0] << 8) | p[1];
    DMSG("Reading 0x%02lx bytes to from offset 0x%04x\n", le, file_offset);

    // No file selected?
    if(!sim->selected_file)
        return apdu_reply_app_error(sim, APDU_STAT_TECH_PROBLEM, 0);

    if(!file_access_allowed(sim, sim->selected_file, FILE_ACTION_READ)) {
        DMSG("Access rights not sufficient\n");
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_ACCESS_COND_NOT_FULFILLED);
    }

    if(file_get_type(sim->selected_file) != FILE_TYPE_EF || file_get_structure(sim->selected_file) != FILE_STRUCTURE_TRANSPARENT) {
        DMSG("Selected file is not a transparent EF\n");
        return apdu_reply_ref_error(sim, APDU_REF_ERR_INCONSISTENT);
    }

    int filesize = file_get_size(sim->selected_file);
    if(file_offset + le > filesize) {
        DMSG("Supplied offset and length is not in file\n");
        return apdu_reply_ref_error(sim, APDU_REF_ERR_OUT_OF_RANGE);
    }

    uint8_t rd_data[256];
    int r = file_read(sim->selected_file, rd_data, file_offset, le);
    if(r < 0) {
        return apdu_reply_mem_error(sim);
    }

    return apdu_reply_success(sim, r, rd_data);
}

struct msg *apdu_update_binary(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le)
{
    uint16_t file_offset = (p[0] << 8) | p[1];
    DMSG("Updating 0x%02lx bytes to at offset 0x%04x\n", lc, file_offset);
    DMSG("New file content: %s\n", util_hexdump(data, lc));

    // No file selected?
    if(!sim->selected_file)
        return apdu_reply_app_error(sim, APDU_STAT_TECH_PROBLEM, 0);

    if(!file_access_allowed(sim, sim->selected_file, FILE_ACTION_UPDATE)) {
        DMSG("Access rights not sufficient\n");
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_ACCESS_COND_NOT_FULFILLED);
    }

    if(file_get_type(sim->selected_file) != FILE_TYPE_EF || file_get_structure(sim->selected_file) != FILE_STRUCTURE_TRANSPARENT)
        return apdu_reply_ref_error(sim, APDU_REF_ERR_INCONSISTENT);

    int filesize = file_get_size(sim->selected_file);
    if(file_offset + lc > filesize) {
        DMSG("Supplied offset and length is not in file\n");
        return apdu_reply_ref_error(sim, APDU_REF_ERR_OUT_OF_RANGE);
    }

    int r = file_write(sim->selected_file, data, file_offset, lc);
    if(r < 0) {
        return apdu_reply_mem_error(sim);
    }

    sim_save_to_file(sim);
    return apdu_reply_success(sim, 0, NULL);
}
