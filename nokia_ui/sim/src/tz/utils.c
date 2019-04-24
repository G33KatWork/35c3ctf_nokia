#include "socket/utils.h"

#include <string.h>
#include <stddef.h>

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

char *util_readfile(const char *filename)
{
    TEE_ObjectHandle object;
    TEE_ObjectInfo object_info;
    TEE_Result res;
    size_t read_bytes;

    res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
                    filename, strlen(filename),
                    TEE_DATA_FLAG_ACCESS_READ |
                    TEE_DATA_FLAG_SHARE_READ,
                    &object);

    if (res != TEE_SUCCESS) {
        EMSG("Failed to open persistent object, res=0x%08x", res);
        return NULL;
    }

    res = TEE_GetObjectInfo1(object, &object_info);
    if (res != TEE_SUCCESS) {
        EMSG("Failed to retrieve object information, res=0x%08x", res);
        goto exit;
    }

    char *content = TEE_Malloc(object_info.dataSize, 0);
    if(!content)
        goto exit;

    res = TEE_ReadObjectData(object, content, object_info.dataSize, &read_bytes);
    if(res != TEE_SUCCESS || object_info.dataSize != read_bytes) {
        EMSG("TEE_ReadObjectData failed 0x%08x, read %u over %u", res, read_bytes, object_info.dataSize);
        goto exit;
    }

    TEE_CloseObject(object);
    return content;

exit:
    TEE_CloseObject(object);
    return NULL;
}

void util_writefile(const char *filename, const char *content)
{
    TEE_ObjectHandle object;
    TEE_Result res;
    uint32_t obj_data_flag;

    obj_data_flag = TEE_DATA_FLAG_ACCESS_READ |        /* we can later read the oject */
            TEE_DATA_FLAG_ACCESS_WRITE |        /* we can later write into the object */
            TEE_DATA_FLAG_ACCESS_WRITE_META |   /* we can later destroy or rename the object */
            TEE_DATA_FLAG_OVERWRITE;        /* destroy existing object of same ID */

    res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE,
                    filename, strlen(filename),
                    obj_data_flag,
                    TEE_HANDLE_NULL,
                    NULL, 0,        /* we may not fill it right now */
                    &object);

    if (res != TEE_SUCCESS) {
        EMSG("TEE_CreatePersistentObject failed 0x%08x", res);
        return;
    }

    res = TEE_WriteObjectData(object, content, strlen(content));
    if (res != TEE_SUCCESS) {
        EMSG("TEE_WriteObjectData failed 0x%08x", res);
        TEE_CloseAndDeletePersistentObject1(object);
    } else {
        TEE_CloseObject(object);
    }
}
