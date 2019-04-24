#ifndef _SCREEN_INPUT_DIALOG_
#define _SCREEN_INPUT_DIALOG_

#include "screen.h"

#include <stdlib.h>

typedef enum {
	INPUT_DLG_TYPE_TEXT,
	INPUT_DLG_TYPE_NUMBER
} input_dialog_type;

struct screen_input_dialog_arguments {
	input_dialog_type type;
	size_t max_input_len;
	char* input_buffer;
	const char* prompt_text;
	const char* button_text;
	const char* button_text_empty;
	struct screen* success_return_screen;
	void* success_return_screen_arguments;
	struct screen* empty_return_screen;
	void* empty_return_screen_arguments;
	struct screen* cancel_return_screen;
	void* cancel_return_screen_arguments;
};

extern struct screen screen_input_dialog;

#endif
