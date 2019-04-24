#include "apdu_replies.h"

#include "logging.h"
#include "apdu.h"

#include <string.h>

struct msg *apdu_reply_success(struct simcard *sim, size_t datalen, uint8_t *data)
{
    struct msg *reply;
    uint8_t *payload;

    if(!data && datalen == 0) {
        reply = msg_alloc(2);
        payload = msg_data(reply);
        payload[0] = APDU_STAT_NORMAL;
        payload[1] = 0;
    } else if(data && datalen > 0) {
        reply = msg_alloc(2 + datalen);
        payload = msg_data(reply);
        memcpy(payload, data, datalen);
        payload += datalen;
        payload[0] = APDU_STAT_NORMAL;
        payload[1] = 0;
    } else if(!data && datalen > 0) {
        reply = msg_alloc(2);
        payload = msg_data(reply);
        payload[0] = APDU_STAT_RESPONSE;
        payload[1] = datalen;
    } else {
        EMSG("Data pointer passed without len - shouldn't happen, not sending reply\n");
        return NULL;
    }

    DMSG("replying success...\n");
    return reply;
}

struct msg *apdu_reply_mem_error(struct simcard *sim)
{
    struct msg *reply;
    uint8_t *payload;

    reply = msg_alloc(2);
    payload = msg_data(reply);
    payload[0] = APDU_STAT_MEM_PROBLEM;
    payload[1] = 0x40;

    DMSG("replying mem error...\n");
    return reply;
}

struct msg *apdu_reply_ref_error(struct simcard *sim, uint8_t reason)
{
    struct msg *reply;
    uint8_t *payload;

    reply = msg_alloc(2);
    payload = msg_data(reply);
    payload[0] = APDU_STAT_REFERENCING;
    payload[1] = reason;

    DMSG("replying ref error...\n");
    return reply;
}

struct msg *apdu_reply_app_error(struct simcard *sim, uint8_t sw1, uint8_t sw2)
{
    struct msg *reply;
    uint8_t *payload;

    reply = msg_alloc(2);
    payload = msg_data(reply);
    payload[0] = sw1;
    payload[1] = sw2;

    DMSG("replying app error...\n");
    return reply;
}
