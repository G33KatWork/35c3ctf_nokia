#include "menus/main_menu.h"

#include "animations.h"

#include "screens/phone_book_browse.h"
#include "screens/phone_book_add.h"
#include "screens/phone_book_delete.h"
#include "screens/sms_write.h"
#include "screens/sms_inbox.h"
#include "screens/template_create.h"
#include "screens/template_manage.h"
#include "screens/main_screen.h"

#include <stdlib.h>

struct menu phone_book_submenu;
struct menu sms_submenu;
struct menu settings_submenu;
struct menu sms_templates_submenu;

//********************** Phone book **********************

struct screen_phone_book_browse_arguments phone_book_browse_arguments = {
	.keep_old_selected_entry_id = false
};

struct menu phone_book_browse_screen = {
	.type = MENU_TYPE_SPECIAL,
	.num_entries = 0,
	.parent_menu = &phone_book_submenu,
	.special_screen = &screen_phone_book_browse,
	.arguments = &phone_book_browse_arguments,
	.entries = NULL
};

struct menu phone_book_add_screen = {
	.type = MENU_TYPE_SPECIAL,
	.num_entries = 0,
	.parent_menu = &phone_book_submenu,
	.special_screen = &screen_phone_book_add,
	.entries = NULL
};

struct menu phone_book_delete_screen = {
	.type = MENU_TYPE_SPECIAL,
	.num_entries = 0,
	.parent_menu = &phone_book_submenu,
	.special_screen = &screen_phone_book_delete,
	.entries = NULL
};

struct menu_entry phone_book_menu_entries[] = {
	{
		.animation = NULL,
		.name = "Browse",
		.sub_menu = &phone_book_browse_screen
	},
	{
		.animation = NULL,
		.name = "Add",
		.sub_menu = &phone_book_add_screen
	},
	{
		.animation = NULL,
		.name = "Delete",
		.sub_menu = &phone_book_delete_screen
	}
};

struct menu phone_book_submenu = {
	.type = MENU_TYPE_SUB,
	.num_entries = sizeof(phone_book_menu_entries) / sizeof(struct menu_entry),
	.parent_menu = &main_menu,
	.entries = phone_book_menu_entries
};


//********************** Messages **********************

struct menu sms_write_screen = {
	.type = MENU_TYPE_SPECIAL,
	.num_entries = 0,
	.parent_menu = &sms_submenu,
	.special_screen = &screen_sms_write,
	.arguments = &screen_sms_write_arguments_default,
	.entries = NULL
};

struct menu sms_inbox_screen = {
	.type = MENU_TYPE_SPECIAL,
	.num_entries = 0,
	.parent_menu = &sms_submenu,
	.special_screen = &screen_sms_inbox,
	.entries = NULL
};

struct menu template_create_screen = {
	.type = MENU_TYPE_SPECIAL,
	.num_entries = 0,
	.parent_menu = &sms_templates_submenu,
	.special_screen = &screen_template_create,
	.entries = NULL
};

struct menu template_manage_screen = {
	.type = MENU_TYPE_SPECIAL,
	.num_entries = 0,
	.parent_menu = &sms_templates_submenu,
	.special_screen = &screen_template_manage,
	.entries = NULL
};

struct menu_entry sms_templates_submenu_entries[] = {
	{
		.animation = NULL,
		.name = "View",
		.sub_menu = &template_manage_screen
	},
	{
		.animation = NULL,
		.name = "Create new",
		.sub_menu = &template_create_screen
	}
};

struct menu sms_templates_submenu = {
	.type = MENU_TYPE_SUB,
	.num_entries = sizeof(sms_templates_submenu_entries) / sizeof(struct menu_entry),
	.parent_menu = &sms_submenu,
	.special_screen = NULL,
	.entries = sms_templates_submenu_entries
};

struct menu_entry sms_menu_entries[] = {
	{
		.animation = NULL,
		.name = "New message",
		.sub_menu = &sms_write_screen
	},
	{
		.animation = NULL,
		.name = "Inbox",
		.sub_menu = &sms_inbox_screen
	},
	{
		.animation = NULL,
		.name = "Templates",
		.sub_menu = &sms_templates_submenu
	}
	/*{
	.animation = NULL,
	.name = "Sent",
	.sub_menu = NULL
	}*/
};

struct menu sms_submenu = {
	.type = MENU_TYPE_SUB,
	.num_entries = sizeof(sms_menu_entries) / sizeof(struct menu_entry),
	.parent_menu = &main_menu,
	.entries = sms_menu_entries
};

//********************** Main menu **********************

struct menu_entry main_menu_entries[] = {
	{
		.animation = &menu_phnone_book_anim,
		.name = "Phone book",
		.sub_menu = &phone_book_submenu
	},
	{
		.animation = &menu_sms_anim,
		.name = "Messages",
		.sub_menu = &sms_submenu
	}
};

struct menu main_menu = {
	.type = MENU_TYPE_MAIN,
	.num_entries = sizeof(main_menu_entries) / sizeof(struct menu_entry),
	.parent_menu = NULL,
	.entries = main_menu_entries,
	.selected_entry = 0,
	.submenu_current_top_item = 0
};


//********************** Main menu instance **********************

struct menu_instance main_menu_instance = {
	.menu = &main_menu,
	.return_screen = &screen_main_screen,
	.return_screen_arguments = NULL
};

//********************** Main menu arguments **********************

struct screen_menu_arguments main_menu_return_same_selection = {
	.menu_instance = &main_menu_instance,
	.item_to_select = -1,
	.submenu_to_set = NULL
};

struct screen_menu_arguments main_menu_select_first_item = {
	.menu_instance = &main_menu_instance,
	.item_to_select = 0,
	.submenu_to_set = &main_menu
};
