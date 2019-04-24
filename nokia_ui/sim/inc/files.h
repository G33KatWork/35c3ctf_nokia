#ifndef _FILES_H_
#define _FILES_H_

#include "sim.h"

#include <stdint.h>

enum FILE_ACTION {
    FILE_ACTION_READ,
    FILE_ACTION_UPDATE,
    FILE_ACTION_INCREASE,
    FILE_ACTION_INVALIDATE,
    FILE_ACTION_REHABILITATE
};

enum FILE_ACCESS_CONDITION {
    FILE_ACCESS_CONDITION_ALW = 0,
    FILE_ACCESS_CONDITION_CHV1 = 1,
    FILE_ACCESS_CONDITION_CHV2 = 2,
    FILE_ACCESS_CONDITION_ADM = 4,
    FILE_ACCESS_CONDITION_NEV = 0xF
};

enum FILE_STRUCTURE {
    FILE_STRUCTURE_TRANSPARENT = 0,
    FILE_STRUCTURE_LINEAR_FIXED = 1,
    FILE_STRUCTURE_CYCLIC = 3
};

enum FILE_TYPE {
    FILE_TYPE_MF = 1,
    FILE_TYPE_DF = 2,
    FILE_TYPE_EF = 4
};

enum FILE_RECORD_MODE {
    FILE_RECORD_MODE_NEXT = 2,
    FILE_RECORD_MODE_PREVIOUS = 3,
    FILE_RECORD_MODE_ABSOLUTE = 4
};

cJSON *file_find(struct simcard *sim, uint16_t id);

enum FILE_TYPE file_get_type(cJSON *file);
enum FILE_STRUCTURE file_get_structure(cJSON *file);

int file_get_id(cJSON *file);
int file_get_size(cJSON *file);
int file_get_num_df(cJSON *file);
int file_get_num_ef(cJSON *file);
bool file_valid(cJSON *file);
bool file_set_valid(cJSON *file, bool valid);
bool file_invalid_allow_rw(cJSON *file);
enum FILE_ACCESS_CONDITION file_get_access_condition(cJSON *file, enum FILE_ACTION ac);
int file_read(cJSON *file, uint8_t *buf, uint16_t offset, size_t len);
int file_write(cJSON *file, uint8_t *buf, uint16_t offset, size_t len);

int file_record_ptr_get(cJSON *file);
int file_record_ptr_set(cJSON *file, uint8_t ptr);
int file_get_num_records(cJSON *file);
int file_read_record(cJSON *file, uint8_t *buf, uint8_t recordnum);
int file_write_record(cJSON *file, uint8_t *buf, uint8_t recordnum);

bool file_access_allowed(struct simcard *sim, cJSON *file, enum FILE_ACTION action);

#endif
