#include "screens/phone_book_browse.h"

#include <string.h>
#include <assert.h>

#include "screen.h"
#include "db.h"

#include "screens/menu.h"
#include "screens/phone_book_browser.h"
#include "screens/main_screen.h"
#include "screens/sms_write.h"

#include "menus/main_menu.h"

static struct screen_menu_arguments phone_book_browse_options_menu_arguments;

static int phone_book_browse_id;
static char phone_book_browse_name[DB_PHONEBOOK_NAME_MAX_LEN+1];
static char phone_book_browse_number[DB_PHONEBOOK_NUMBER_MAX_LEN+1];

static void phone_book_browse_screen_enter(struct nokia_ui *ui, void* arguments)
{
	assert(arguments);
	struct screen_phone_book_browse_arguments* browse_arguments = arguments;

	memset(phone_book_browse_name, 0, sizeof(phone_book_browse_name));
	memset(phone_book_browse_number, 0, sizeof(phone_book_browse_number));

	screen_set_current(ui, &screen_phone_book_browser, &(struct screen_phone_book_browser_arguments){
		.selected_entry_id = &phone_book_browse_id,
		.keep_old_selected_entry_id = browse_arguments->keep_old_selected_entry_id,

		.max_name_len = sizeof(phone_book_browse_name) - 1,
		.name_buffer = phone_book_browse_name,

		.max_number_len = sizeof(phone_book_browse_number) - 1,
		.number_buffer = phone_book_browse_number,

		.button_text = "Options",

		.success_return_screen = &screen_menu,
		.success_return_screen_arguments = &phone_book_browse_options_menu_arguments,

		.cancel_return_screen = &screen_menu,
		.cancel_return_screen_arguments = &main_menu_return_same_selection
	});
}

struct screen screen_phone_book_browse = {
	.enter_callback = phone_book_browse_screen_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = screen_draw_null,
	.handle_input = screen_handle_input_null,
	.handle_baseband = screen_handle_baseband_null,
};




struct menu phone_book_browse_options_menu;

struct screen_sms_write_arguments sms_write_arguments_write_sms = {
	.phone_number = phone_book_browse_number
};

struct menu phone_book_browse_sms_send_screen = {
	.type = MENU_TYPE_SPECIAL,
	.num_entries = 0,
	.parent_menu = &phone_book_browse_options_menu,
	.special_screen = &screen_sms_write,
	.arguments = &sms_write_arguments_write_sms,
	.entries = NULL
};

struct menu_entry phone_book_browse_options_menu_entries[] = {
	{
		.animation = NULL,
		.name = "Write SMS",
		.sub_menu = &phone_book_browse_sms_send_screen
	}
};

struct menu phone_book_browse_options_menu = {
	.type = MENU_TYPE_SUB,
	.num_entries = 1,
	.parent_menu = NULL,
	.entries = phone_book_browse_options_menu_entries,
	.selected_entry = 0,
	.submenu_current_top_item = 0
};

struct screen_phone_book_browse_arguments phone_book_browse_return_screen_arguments = {
	.keep_old_selected_entry_id = true
};

struct menu_instance phone_book_browse_options_menu_instance = {
	.menu = &phone_book_browse_options_menu,
	.return_screen = &screen_phone_book_browse,
	.return_screen_arguments = &phone_book_browse_return_screen_arguments
};


static struct screen_menu_arguments phone_book_browse_options_menu_arguments = {
	.menu_instance = &phone_book_browse_options_menu_instance,
	.item_to_select = 0
};
