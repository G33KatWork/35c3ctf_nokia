#ifndef _PHONE_BOOK_BROWSE_SCREEN_
#define _PHONE_BOOK_BROWSE_SCREEN_

#include "screen.h"

#include <stdbool.h>

struct screen_phone_book_browse_arguments {
    _Bool keep_old_selected_entry_id;
};

extern struct screen screen_phone_book_browse;

#endif
