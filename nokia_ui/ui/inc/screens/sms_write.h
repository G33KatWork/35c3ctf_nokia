#ifndef _SCREEN_SMS_WRITE_
#define _SCREEN_SMS_WRITE_

#include "screen.h"

struct screen_sms_write_arguments {
    char* phone_number;
};

extern struct screen screen_sms_write;

extern struct screen_sms_write_arguments screen_sms_write_arguments_default;

#endif
