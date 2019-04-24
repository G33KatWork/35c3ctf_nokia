#ifndef _MENU_SCREEN_
#define _MENU_SCREEN_

#include "screen.h"
#include "animations.h"

#include <stdbool.h>

struct menu;

enum {
	MENU_TYPE_MAIN,
	MENU_TYPE_SUB,
	MENU_TYPE_SPECIAL
};

struct menu_entry {
	struct animation* animation;
	char* name;
	struct menu* sub_menu;
};

struct menu {
	int type;
	int num_entries;
	struct menu* parent_menu;
	struct screen* special_screen;
	void* arguments;
	struct menu_entry* entries;
	int selected_entry;
	int submenu_current_top_item;
};

struct menu_instance {
	struct menu* menu;
	struct screen* return_screen;
	void* return_screen_arguments;
};

extern struct screen screen_menu;

struct screen_menu_arguments {
	struct menu_instance* menu_instance;
	struct menu* submenu_to_set;
	int item_to_select;
};

#endif
