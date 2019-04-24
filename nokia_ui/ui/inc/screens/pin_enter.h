#ifndef _SCREEN_PIN_ENTER_
#define _SCREEN_PIN_ENTER_

#include "screen.h"

#include <stdlib.h>

struct screen_pin_enter_arguments {
    size_t max_input_len;
    size_t min_input_len;
    char* input_buffer;
    const char* line1_text;
    const char* line2_text;
    const char* button_text;
    int* entered_text_len;
    struct screen* success_return_screen;
    void* success_return_screen_arguments;
};

extern struct screen screen_pin_enter;

#endif
