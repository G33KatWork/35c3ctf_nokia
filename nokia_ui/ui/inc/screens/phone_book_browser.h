#ifndef _SCREEN_PHONE_BOOK_BROWSER_
#define _SCREEN_PHONE_BOOK_BROWSER_

#include "screen.h"

#include <stdlib.h>
#include <stdbool.h>

struct screen_phone_book_browser_arguments {
	int* selected_entry_id;
	_Bool keep_old_selected_entry_id;
	size_t max_name_len;
	char* name_buffer;
	size_t max_number_len;
	char* number_buffer;

	const char* button_text;

	struct screen* success_return_screen;
	void* success_return_screen_arguments;

	struct screen* cancel_return_screen;
	void* cancel_return_screen_arguments;
};

extern struct screen screen_phone_book_browser;

#endif
