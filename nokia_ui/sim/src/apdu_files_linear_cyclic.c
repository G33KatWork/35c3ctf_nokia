#include "apdu_files_linear_cyclic.h"

#include "logging.h"
#include "apdu.h"
#include "apdu_replies.h"
#include "files.h"
#include "utils.h"

struct msg *apdu_read_record(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le)
{
    uint8_t record_num = p[0];
    uint8_t mode = p[1];

    DMSG("Reading 0x%02lx bytes from record number 0x%02x in mode 0x%02x\n", le, record_num, mode);

    if(mode != FILE_RECORD_MODE_NEXT && mode != FILE_RECORD_MODE_PREVIOUS && mode != FILE_RECORD_MODE_ABSOLUTE) {
        DMSG("Invalid mode supplied\n");
        return apdu_reply_app_error(sim, APDU_STAT_INCORR_P1_P2, 0);
    }

    enum FILE_STRUCTURE file_structure = file_get_structure(sim->selected_file);
    int num_records = file_get_num_records(sim->selected_file);

    // No file selected?
    if(!sim->selected_file)
        return apdu_reply_app_error(sim, APDU_STAT_TECH_PROBLEM, 0);

    if(!file_access_allowed(sim, sim->selected_file, FILE_ACTION_READ)) {
        DMSG("Access rights not sufficient\n");
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_ACCESS_COND_NOT_FULFILLED);
    }

    if(file_get_type(sim->selected_file) != FILE_TYPE_EF || (file_structure != FILE_STRUCTURE_LINEAR_FIXED && file_structure != FILE_STRUCTURE_CYCLIC)) {
        DMSG("Selected file is not a linear fixed or cyclic EF\n");
        return apdu_reply_ref_error(sim, APDU_REF_ERR_INCONSISTENT);
    }

    int filesize = file_get_size(sim->selected_file);
    if(le != filesize) {
        DMSG("Data requested doesn't match filesize. Filesize: 0x%04x - Requested: 0x%04lx\n", filesize, le);
        return apdu_reply_ref_error(sim, APDU_REF_ERR_OUT_OF_RANGE);
    }

    if(mode == FILE_RECORD_MODE_ABSOLUTE && record_num > 0) {
        if(record_num > num_records) {
            DMSG("Trying to absolute read record 0x%04x which doesn't exist\n", record_num);
            return apdu_reply_ref_error(sim, APDU_REF_ERR_OUT_OF_RANGE);
        }
    }

    uint8_t rd_data[256];
    int r, cur_record;

    cur_record = file_record_ptr_get(sim->selected_file);
    if(cur_record == -1) {
        return apdu_reply_mem_error(sim);
    }

    switch(mode) {
        case FILE_RECORD_MODE_ABSOLUTE:
            if(record_num == 0)
                r = file_read_record(sim->selected_file, rd_data, cur_record);
            else
                r = file_read_record(sim->selected_file, rd_data, record_num-1);
            break;

        case FILE_RECORD_MODE_NEXT:
            if(file_structure == FILE_STRUCTURE_LINEAR_FIXED) {
                if(cur_record == num_records-1) {
                    DMSG("Trying to read next record in linear fixed file while pointer already points to the last entry\n");
                    return apdu_reply_ref_error(sim, APDU_REF_ERR_OUT_OF_RANGE);
                }

                cur_record++;
            } else if(file_structure == FILE_STRUCTURE_CYCLIC) {
                cur_record++;
                cur_record %= num_records;
            }

            r = file_read_record(sim->selected_file, rd_data, cur_record);
            if(r >= 0) {
                file_record_ptr_set(sim->selected_file, cur_record);
                sim_save_to_file(sim);
            }

            break;

        case FILE_RECORD_MODE_PREVIOUS:
            if(file_structure == FILE_STRUCTURE_LINEAR_FIXED) {
                if(cur_record == 0) {
                    DMSG("Trying to read previous record in linear fixed file while pointer already points to the first entry\n");
                    return apdu_reply_ref_error(sim, APDU_REF_ERR_OUT_OF_RANGE);
                }

                cur_record--;
            } else if(file_structure == FILE_STRUCTURE_CYCLIC) {
                if(cur_record == 0) cur_record = num_records-1;
                else cur_record--;
            }

            r = file_read_record(sim->selected_file, rd_data, cur_record);
            if(r >= 0) {
                file_record_ptr_set(sim->selected_file, cur_record);
                sim_save_to_file(sim);
            }

            break;
    }

    return apdu_reply_success(sim, r, rd_data);
}

struct msg *apdu_update_record(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le)
{
    uint8_t record_num = p[0];
    uint8_t mode = p[1];

    DMSG("Updating 0x%02lx bytes from record number 0x%02x in mode 0x%02x\n", lc, record_num, mode);
    DMSG("New file content: %s\n", util_hexdump(data, lc));

    if(mode != FILE_RECORD_MODE_NEXT && mode != FILE_RECORD_MODE_PREVIOUS && mode != FILE_RECORD_MODE_ABSOLUTE) {
        DMSG("Invalid mode supplied\n");
        return apdu_reply_app_error(sim, APDU_STAT_INCORR_P1_P2, 0);
    }

    enum FILE_STRUCTURE file_structure = file_get_structure(sim->selected_file);
    int num_records = file_get_num_records(sim->selected_file);

    // No file selected?
    if(!sim->selected_file)
        return apdu_reply_app_error(sim, APDU_STAT_TECH_PROBLEM, 0);

    if(!file_access_allowed(sim, sim->selected_file, FILE_ACTION_UPDATE)) {
        DMSG("Access rights not sufficient\n");
        return apdu_reply_app_error(sim, APDU_STAT_SECURITY, APDU_SEC_ERR_ACCESS_COND_NOT_FULFILLED);
    }

    if(file_get_type(sim->selected_file) != FILE_TYPE_EF || (file_structure != FILE_STRUCTURE_LINEAR_FIXED && file_structure != FILE_STRUCTURE_CYCLIC)) {
        DMSG("Selected file is not a linear fixed or cyclic EF\n");
        return apdu_reply_ref_error(sim, APDU_REF_ERR_INCONSISTENT);
    }

    int filesize = file_get_size(sim->selected_file);
    if(lc != filesize) {
        DMSG("Data supplied doesn't match filesize. Filesize: 0x%04x - Supplied: 0x%04lx\n", filesize, lc);
        return apdu_reply_ref_error(sim, APDU_REF_ERR_OUT_OF_RANGE);
    }

    if(mode == FILE_RECORD_MODE_ABSOLUTE && record_num > 0) {
        if(record_num > num_records) {
            DMSG("Trying to absolute write record 0x%04x which doesn't exist\n", record_num);
            return apdu_reply_ref_error(sim, APDU_REF_ERR_OUT_OF_RANGE);
        }
    }

    int r, cur_record;

    cur_record = file_record_ptr_get(sim->selected_file);
    if(cur_record == -1) {
        return apdu_reply_mem_error(sim);
    }

    switch(mode) {
        case FILE_RECORD_MODE_ABSOLUTE:
            if(record_num == 0)
                r = file_write_record(sim->selected_file, data, cur_record);
            else
                r = file_write_record(sim->selected_file, data, record_num-1);
            break;

        case FILE_RECORD_MODE_NEXT:
            if(file_structure == FILE_STRUCTURE_LINEAR_FIXED) {
                if(cur_record == num_records-1) {
                    DMSG("Trying to write next record in linear fixed file while pointer already points to the last entry\n");
                    return apdu_reply_ref_error(sim, APDU_REF_ERR_OUT_OF_RANGE);
                }

                cur_record++;
            } else if(file_structure == FILE_STRUCTURE_CYCLIC) {
                cur_record++;
                cur_record %= num_records;
            }

            r = file_write_record(sim->selected_file, data, cur_record);
            if(r >= 0) {
                file_record_ptr_set(sim->selected_file, cur_record);
                sim_save_to_file(sim);
            }

            break;

        case FILE_RECORD_MODE_PREVIOUS:
            if(file_structure == FILE_STRUCTURE_LINEAR_FIXED) {
                if(cur_record == 0) {
                    DMSG("Trying to read previous record in linear fixed file while pointer already points to the first entry\n");
                    return apdu_reply_ref_error(sim, APDU_REF_ERR_OUT_OF_RANGE);
                }

                cur_record--;
            } else if(file_structure == FILE_STRUCTURE_CYCLIC) {
                if(cur_record == 0) cur_record = num_records-1;
                else cur_record--;
            }

            r = file_write_record(sim->selected_file, data, cur_record);
            if(r >= 0) {
                file_record_ptr_set(sim->selected_file, cur_record);
                sim_save_to_file(sim);
            }

            break;
    }

    return apdu_reply_success(sim, 0, NULL);
}

//struct msg *apdu_seek(struct simcard *sim, uint8_t *p, size_t lc, uint8_t *data, size_t le)
//{
//    uint8_t type = (p[1] & 0xF0) >> 4;
//    uint8_t mode = p[1] & 0x0F;
//
//    if(type != 0 && type != 1)
//        goto out_err;
//
//    if(lc == 0)
//        goto out_err;
//
//    if(type == 1 && le != 0)
//        goto out_err;
//
//    printf("Seek command type 0x%02x, mode 0x%02x, pattern length 0x%02lx\n", type, mode, lc);
//    printf("Pattern: %s\n", util_hexdump(data, lc));
//
//    if(type == 1)
//        printf("Type 2 SEEK command, record number is requested (len 0x%02lx)\n", le);
//
//out_err:
//    return NULL;
//}
