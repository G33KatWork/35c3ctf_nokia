#ifndef _INFO_SCREEN_
#define _INFO_SCREEN_

#include "animations.h"
#include "screen.h"

struct screen_info_arguments {
	const char* text_line1;
	const char* text_line2;
	const char* text_line3;
	struct animation* animation;
	struct screen* return_screen;
	void* return_screen_arguments;
};

extern struct screen screen_info;

#endif
