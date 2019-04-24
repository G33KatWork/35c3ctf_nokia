#include "screens/phone_book_add.h"

#include "db.h"
#include "screen.h"
#include "screens/input_dialog.h"
#include "screens/info.h"
#include "screens/menu.h"

#include "menus/main_menu.h"

#include <string.h>
#include <alloca.h>

struct screen screen_phone_book_add_step2;
struct screen screen_phone_book_add_step3;

static char phone_book_add_name[DB_PHONEBOOK_NAME_MAX_LEN+1];
static char phone_book_add_number[DB_PHONEBOOK_NUMBER_MAX_LEN+1];

static void phone_book_add_screen_enter(struct nokia_ui *ui, void* arguments)
{
	memset(phone_book_add_name, 0, sizeof(phone_book_add_name));
	memset(phone_book_add_number, 0, sizeof(phone_book_add_number));

	screen_set_current(ui, &screen_input_dialog, &(struct screen_input_dialog_arguments){
		.type = INPUT_DLG_TYPE_TEXT,
		.max_input_len = sizeof(phone_book_add_name) - 1,
		.input_buffer = phone_book_add_name,
		.prompt_text = "Name:",
		.button_text = "OK",
		.button_text_empty = "OK",

		.success_return_screen = &screen_phone_book_add_step2,
		.success_return_screen_arguments = NULL,

		.empty_return_screen = &screen_phone_book_add_step2,
		.empty_return_screen_arguments = NULL,

		.cancel_return_screen = &screen_menu,
		.cancel_return_screen_arguments = &main_menu_return_same_selection
	});
}

struct screen screen_phone_book_add = {
	.enter_callback = phone_book_add_screen_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = screen_draw_null,
	.handle_input = screen_handle_input_null,
	.handle_baseband = screen_handle_baseband_null,
};




static void phone_book_add_step2_screen_enter(struct nokia_ui *ui, void* arguments)
{
	screen_set_current(ui, &screen_input_dialog, &(struct screen_input_dialog_arguments){
		.type = INPUT_DLG_TYPE_NUMBER,
		.max_input_len = sizeof(phone_book_add_number) - 1,
		.input_buffer = phone_book_add_number,
		.prompt_text = "Number:",
		.button_text = "OK",
		.button_text_empty = "OK",

		.success_return_screen = &screen_phone_book_add_step3,
		.success_return_screen_arguments = NULL,

		.empty_return_screen = &screen_phone_book_add_step3,
		.empty_return_screen_arguments = NULL,

		.cancel_return_screen = &screen_menu,
		.cancel_return_screen_arguments = &main_menu_return_same_selection
	});
}

struct screen screen_phone_book_add_step2 = {
	.enter_callback = phone_book_add_step2_screen_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = screen_draw_null,
	.handle_input = screen_handle_input_null,
	.handle_baseband = screen_handle_baseband_null,
};




static void phone_book_add_step3_screen_enter(struct nokia_ui *ui, void* arguments)
{
	db_add_phone_book_entry(phone_book_add_name, phone_book_add_number);

	screen_set_current(ui, &screen_info, &(struct screen_info_arguments){
		.text_line1 = "Saved",
		.text_line2 = "",
		.text_line3 = "",
		.animation = &success_anim,
		.return_screen = &screen_menu,
		.return_screen_arguments = &main_menu_return_same_selection
	});
}

struct screen screen_phone_book_add_step3 = {
	.enter_callback = phone_book_add_step3_screen_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = screen_draw_null,
	.handle_input = screen_handle_input_null,
	.handle_baseband = screen_handle_baseband_null,
};
