#ifndef _SCREEN_TEMPLATE_VIEWER_
#define _SCREEN_TEMPLATE_VIEWER_

#include "screen.h"

struct template_viewer_arguments {
	char* text;

	const char* button_text;

	struct screen* return_screen;
	void* return_screen_arguments;

	struct screen* button_screen;
	void* button_screen_arguments;
};

extern struct screen screen_template_viewer;

#endif
