#include "screens/phone_book_delete.h"

#include <string.h>

#include "screen.h"
#include "animations.h"
#include "db.h"

#include "screens/menu.h"
#include "screens/phone_book_browser.h"
#include "screens/info.h"

#include "menus/main_menu.h"

struct screen screen_phone_book_perform_deletion;

static int phone_book_browse_id;
static char phone_book_browse_name[17];
static char phone_book_browse_number[21];

static void phone_book_delete_screen_enter(struct nokia_ui *ui, void* arguments)
{
	memset(phone_book_browse_name, 0, sizeof(phone_book_browse_name));
	memset(phone_book_browse_number, 0, sizeof(phone_book_browse_number));

	screen_set_current(ui, &screen_phone_book_browser, &(struct screen_phone_book_browser_arguments){
		.selected_entry_id = &phone_book_browse_id,

		.max_name_len = sizeof(phone_book_browse_name) - 1,
		.name_buffer = phone_book_browse_name,

		.max_number_len = sizeof(phone_book_browse_number) - 1,
		.number_buffer = phone_book_browse_number,

		.button_text = "Delete",

		.success_return_screen = &screen_phone_book_perform_deletion,
		.success_return_screen_arguments = NULL,

		.cancel_return_screen = &screen_menu,
		.cancel_return_screen_arguments = &main_menu_return_same_selection
	});
}

struct screen screen_phone_book_delete = {
	.enter_callback = phone_book_delete_screen_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = screen_draw_null,
	.handle_input = screen_handle_input_null,
	.handle_baseband = screen_handle_baseband_null,
};




static void phone_book_perform_deletion_screen_enter(struct nokia_ui *ui, void* arguments)
{
	db_del_phone_book_entry(phone_book_browse_id);

	screen_set_current(ui, &screen_info, &(struct screen_info_arguments){
		.text_line1 = "Memory",
		.text_line2 = "erased",
		.text_line3 = "",
		.animation = &erase_anim,
		.return_screen = &screen_menu,
		.return_screen_arguments = &main_menu_return_same_selection
	});
}

struct screen screen_phone_book_perform_deletion = {
	.enter_callback = phone_book_perform_deletion_screen_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = screen_draw_null,
	.handle_input = screen_handle_input_null,
	.handle_baseband = screen_handle_baseband_null,
};
