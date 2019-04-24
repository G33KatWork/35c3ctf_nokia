#ifndef _SCREEN_TEXT_EDITOR_
#define _SCREEN_TEXT_EDITOR_

#include "screen.h"

#include <stdlib.h>

struct screen_text_editor_arguments {
	size_t max_input_len;
    char* input_buffer;
    int* entered_text_len;

    bool sms_enable_multipart;
	
    const char* button_text;
	struct screen* success_return_screen;
	void* success_return_screen_arguments;
	struct screen* cancel_return_screen;
	void* cancel_return_screen_arguments;
};

extern struct screen screen_text_editor;

#endif
