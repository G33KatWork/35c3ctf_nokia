#ifndef _ERROR_SCREEN_
#define _ERROR_SCREEN_

#include "animations.h"
#include "screen.h"

struct screen_error_arguments {
    const char* text_line1;
    const char* text_line2;
    const char* text_line3;
};

extern struct screen screen_error;

#endif
