#include "files.h"

#include "logging.h"
#include "utils.h"
#include "chv.h"

#include <string.h>

static const char *ac_to_str[] = {
    [FILE_ACTION_READ] = "read",
    [FILE_ACTION_UPDATE] = "update",
    [FILE_ACTION_INCREASE] = "increase",
    [FILE_ACTION_INVALIDATE] = "invalidate",
    [FILE_ACTION_REHABILITATE] = "rehabilitate"
};

static cJSON *file_get_mf(struct simcard *sim)
{
    cJSON *files = cJSON_GetObjectItem(sim->json_simdata, "files");
    if(!files) {
        DMSG("can't find node files in JSON\n");
        return NULL;
    }

    cJSON *mf = cJSON_GetObjectItem(files, "MF");
    return mf;
}

static cJSON *file_get_recurse(cJSON *file, uint16_t id)
{
    cJSON *name = cJSON_GetObjectItem(file, "name");
    if(!name || !cJSON_IsNumber(name)) {
        DMSG("File has no proper name!?\n");
        return NULL;
    }

    if(name->valueint == id)
        return file;

    cJSON *children = cJSON_GetObjectItem(file, "children");
    if(!children || !cJSON_IsObject(children)) {
        return NULL;
    }

    cJSON *child;
    cJSON *found_file;
    cJSON_ArrayForEach(child, children) {
        found_file = file_get_recurse(child, id);
        if(found_file)
            return found_file;
    }

    return NULL;
}

cJSON *file_find(struct simcard *sim, uint16_t id)
{
    DMSG("Searching for file 0x%04x\n", id);

    cJSON *mf = file_get_mf(sim);
    if(!mf) {
        DMSG("can't find MF\n");
        return NULL;
    }

    cJSON *file = file_get_recurse(mf, id);
    if(!file) {
        DMSG("Couldn't find file with ID 0x%04x\n", id);
        return NULL;
    }

    return file;
}

enum FILE_TYPE file_get_type(cJSON *file)
{
    cJSON *file_type = cJSON_GetObjectItem(file, "file_type");
    if(!file_type || !cJSON_IsNumber(file_type)) {
        DMSG("File has no proper file_type value\n");
        return -1;
    }

    return file_type->valueint;
}

enum FILE_STRUCTURE file_get_structure(cJSON *file)
{
    cJSON *file_structure = cJSON_GetObjectItem(file, "file_structure");
    if(!file_structure || !cJSON_IsNumber(file_structure)) {
        DMSG("File has no proper file_structure value\n");
        return -1;
    }

    return file_structure->valueint;
}

int file_get_id(cJSON *file)
{
    cJSON *name = cJSON_GetObjectItem(file, "name");
    if(!name || !cJSON_IsNumber(name)) {
        DMSG("File has no proper name value\n");
        return 0;
    }

    return name->valueint;
}

int file_get_size(cJSON *file)
{
    cJSON *size = cJSON_GetObjectItem(file, "size");
    if(!size || !cJSON_IsNumber(size)) {
        DMSG("File has no proper size value\n");
        return 0;
    }

    return size->valueint;
}

int file_get_num_df(cJSON *file)
{
    int i = 0;
    cJSON *children = cJSON_GetObjectItem(file, "children");

    if(!children || !cJSON_IsObject(children))
        return 0;

    cJSON *child;
    cJSON_ArrayForEach(child, children) {
        enum FILE_TYPE filetype = file_get_type(child);
        if(filetype == FILE_TYPE_DF)
            i++;
    }

    return i;
}

int file_get_num_ef(cJSON *file)
{
    int i = 0;
    cJSON *children = cJSON_GetObjectItem(file, "children");

    if(!children || !cJSON_IsObject(children))
        return 0;

    cJSON *child;
    cJSON_ArrayForEach(child, children) {
        enum FILE_TYPE filetype = file_get_type(child);
        if(filetype == FILE_TYPE_EF)
            i++;
    }

    return i;
}

bool file_valid(cJSON *file)
{
    cJSON *valid = cJSON_GetObjectItem(file, "valid");
    if(!valid || !cJSON_IsBool(valid)) {
        DMSG("File has no proper valid value\n");
        return false;
    }

    return cJSON_IsTrue(valid);
}

bool file_set_valid(cJSON *file, bool valid)
{
    cJSON *valid_json = cJSON_GetObjectItem(file, "valid");
    if(!valid_json || !cJSON_IsBool(valid_json)) {
        DMSG("File has no proper valid value\n");
        return false;
    }

    cJSON_ReplaceItemInObject(file, "valid", cJSON_CreateBool(valid));

    return true;
}

bool file_invalid_allow_rw(cJSON *file)
{
    cJSON *invalid_allow_rw = cJSON_GetObjectItem(file, "invalid_allow_rw");
    if(!invalid_allow_rw || !cJSON_IsBool(invalid_allow_rw)) {
        DMSG("File has no proper invalid_allow_rw value\n");
        return false;
    }

    return cJSON_IsTrue(invalid_allow_rw);
}

enum FILE_ACCESS_CONDITION file_get_access_condition(cJSON *file, enum FILE_ACTION ac)
{
    cJSON *access = cJSON_GetObjectItem(file, "access");

    if(!access || !cJSON_IsObject(access)) {
        DMSG("file object has no access object, assuming all access conditions to be NEV\n");
        return FILE_ACCESS_CONDITION_NEV;
    }

    cJSON *access_cond = cJSON_GetObjectItem(access, ac_to_str[ac]);
    if(!access_cond || !cJSON_IsNumber(access_cond)) {
        DMSG("file access condition %s not present or not a number, assuming access condition NEV\n", ac_to_str[ac]);
        return FILE_ACCESS_CONDITION_NEV;
    }

    return access_cond->valueint;
}

int file_record_ptr_get(cJSON *file)
{
    cJSON *record_ptr = cJSON_GetObjectItem(file, "record_ptr");

    if(!record_ptr || !cJSON_IsNumber(record_ptr)) {
        DMSG("file object has no record_ptr\n");
        return -1;
    }

    return record_ptr->valueint;
}

int file_record_ptr_set(cJSON *file, uint8_t ptr)
{
    cJSON *record_ptr = cJSON_GetObjectItem(file, "record_ptr");

    if(!record_ptr || !cJSON_IsNumber(record_ptr)) {
        DMSG("file object has no record_ptr\n");
        return -1;
    }

    if(ptr >= file_get_num_records(file)) {
        DMSG("record pointer to be set is out of range: 0x%04x\n", ptr);
        return -1;
    }

    cJSON_SetNumberValue(record_ptr, ptr);

    return record_ptr->valueint;
}

int file_get_num_records(cJSON *file)
{
    cJSON *records = cJSON_GetObjectItem(file, "records");

    if(!records || !cJSON_IsArray(records)) {
        DMSG("file object has no records array\n");
        return -1;
    }

    return cJSON_GetArraySize(records);
}

int file_read_record(cJSON *file, uint8_t *buf, uint8_t recordnum)
{
    cJSON *records = cJSON_GetObjectItem(file, "records");

    if(!records || !cJSON_IsArray(records)) {
        DMSG("file object has no records array\n");
        return -1;
    }

    if(recordnum >= file_get_num_records(file)) {
        DMSG("trying to read file record 0x%04x which doesn't exist\n", recordnum);
        return -1;
    }

    int record_size = file_get_size(file);

    cJSON *target_record = cJSON_GetArrayItem(records, recordnum);
    util_unhexlify(buf, cJSON_GetStringValue(target_record), record_size);

    return record_size;
}

int file_write_record(cJSON *file, uint8_t *buf, uint8_t recordnum)
{
    cJSON *records = cJSON_GetObjectItem(file, "records");

    if(!records || !cJSON_IsArray(records)) {
        DMSG("file object has no records array\n");
        return -1;
    }

    if(recordnum >= file_get_num_records(file)) {
        DMSG("trying to write file record 0x%04x which doesn't exist\n", recordnum);
        return -1;
    }

    int record_size = file_get_size(file);

    cJSON *target_record = cJSON_GetArrayItem(records, recordnum);
    util_hexlify(cJSON_GetStringValue(target_record), buf, file_get_size(file));

    return record_size;
}

int file_read(cJSON *file, uint8_t *buf, uint16_t offset, size_t len)
{
    cJSON *value = cJSON_GetObjectItem(file, "value");
    if(!cJSON_IsString(value)) {
        DMSG("File value is not a string\n");
        return -1;
    }

    DMSG("Reading from file offset 0x%04x len 0x%04lx\n", offset, len);

    size_t filesize = strlen(cJSON_GetStringValue(value))/2;
    DMSG("Total filesize is 0x%04lx\n", filesize);

    if(offset >= filesize) {
        DMSG("Offset (0x%04x) exceeds filesize (0x%04lx)\n", offset, filesize);
        return -1;
    }

    size_t read_size = len;
    if(read_size > filesize - offset) read_size = filesize - offset;

    DMSG("Final read size is 0x%04lx\n", read_size);

    util_unhexlify(buf, &(cJSON_GetStringValue(value)[offset*2]), read_size);

    return read_size;
}

int file_write(cJSON *file, uint8_t *buf, uint16_t offset, size_t len)
{
    cJSON *value = cJSON_GetObjectItem(file, "value");
    if(!cJSON_IsString(value)) {
        DMSG("File value is not a string\n");
        return -1;
    }

    DMSG("Writing to file offset 0x%04x len 0x%04lx\n", offset, len);

    size_t filesize = strlen(cJSON_GetStringValue(value))/2;
    DMSG("Total filesize is 0x%04lx\n", filesize);

    if(offset >= filesize) {
        DMSG("Offset (0x%04x) exceeds filesize (0x%04lx)\n", offset, filesize);
        return -1;
    }

    size_t write_size = len;
    if(write_size > filesize - offset) write_size = filesize - offset;

    DMSG("Final write size is 0x%04lx\n", write_size);

    util_hexlify(&(cJSON_GetStringValue(value)[offset*2]), buf, write_size);

    return 0;
}

bool file_access_allowed(struct simcard *sim, cJSON *file, enum FILE_ACTION action)
{
    enum FILE_ACCESS_CONDITION cond = file_get_access_condition(file, action);

    switch(cond) {
        case FILE_ACCESS_CONDITION_ALW:
            return true;
        case FILE_ACCESS_CONDITION_CHV1:
            return !chv_is_enabled(sim, CHV_1) || sim->chv_unlocked[CHV_1];
        case FILE_ACCESS_CONDITION_CHV2:
            return !chv_is_enabled(sim, CHV_2) || sim->chv_unlocked[CHV_2];
        case FILE_ACCESS_CONDITION_ADM:
        case FILE_ACCESS_CONDITION_NEV:
        default:
            return false;
    }
}
