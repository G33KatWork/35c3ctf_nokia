#ifndef _SCREEN_SMS_INBOX_LIST_
#define _SCREEN_SMS_INBOX_LIST_

#include "screen.h"

struct screen_sms_inbox_list_arguments {
	const char* button_text;
	struct screen* button_return_screen;
	void* button_return_screen_arguments;
	struct screen* return_screen;
	void* return_screen_arguments;
};

extern struct screen screen_sms_inbox_list;

#endif
