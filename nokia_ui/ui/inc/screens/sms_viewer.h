#ifndef _SCREEN_SMS_VIEWER_
#define _SCREEN_SMS_VIEWER_

#include "screen.h"

struct sms_viewer_arguments {
	char* text;
	char* number;
    char* sender_name;
	int* date;

	const char* button_text;

	struct screen* return_screen;
	void* return_screen_arguments;

	struct screen* button_screen;
	void* button_screen_arguments;
};

extern struct screen screen_sms_viewer;

#endif
