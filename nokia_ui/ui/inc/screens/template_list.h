#ifndef _SCREEN_TEMPLATE_LIST_
#define _SCREEN_TEMPLATE_LIST_

#include "screen.h"

struct screen_template_list_arguments {
	const char* button_text;
	int* selected_template_id;
	int* selected_template_index;
	struct screen* button_return_screen;
	void* button_return_screen_arguments;
	struct screen* return_screen;
	void* return_screen_arguments;
};

extern struct screen screen_template_list;

#endif
