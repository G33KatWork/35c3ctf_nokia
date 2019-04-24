#ifndef _PROGRESS_SCREEN_
#define _PROGRESS_SCREEN_

#include "screen.h"

struct screen_progress_arguments {
    const char* text_line1;
    const char* text_line2;
    const char* text_button;

    struct screen* button_screen;
    void* button_screen_arguments;

    void (*handle_baseband_cb)(struct nokia_ui *ui, int event, void* data1, void* data2);
};

extern struct screen screen_progress;

#endif
