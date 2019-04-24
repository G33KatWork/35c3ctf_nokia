#include "screens/sms_write.h"

#include "db.h"
#include "screen.h"
#include "uictl.h"
#include "ui_event.h"

#include "generated/font_small_bold.h"
#include "generated/font_large_bold.h"
#include "generated/nokia_images.h"

#include "screens/menu.h"
#include "screens/text_editor.h"
#include "screens/input_dialog.h"
#include "screens/info.h"
#include "screens/progress.h"
#include "screens/phone_book_browser.h"
#include "screens/template_list.h"

#include "menus/main_menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define SMS_MAX_LEN		459

static struct screen_menu_arguments sms_write_options_menu_arguments;

struct screen screen_sms_write_show_editor;
struct screen screen_sms_write_enter_number;
struct screen screen_sms_write_send_sms;
struct screen screen_sms_write_choose_address;
struct screen screen_sms_write_insert_selected_template;

static int phone_book_browse_id;
static char phone_book_browse_name[DB_PHONEBOOK_NAME_MAX_LEN+1];
static char phone_book_browse_number[DB_PHONEBOOK_NUMBER_MAX_LEN+1];
static int num_sms;
static int text_len;
static char text[SMS_MAX_LEN+1] = "TEST TEST";


struct screen_input_dialog_arguments number_input_dialog_arguments = {
	.type = INPUT_DLG_TYPE_NUMBER,
	.max_input_len = sizeof(phone_book_browse_number) - 1,
	.input_buffer = phone_book_browse_number,
	.prompt_text = "Number:",
	.button_text = "OK",
	.button_text_empty = "Search",

	.success_return_screen = &screen_sms_write_send_sms,
	.success_return_screen_arguments = NULL,

	.empty_return_screen = &screen_sms_write_choose_address,
	.empty_return_screen_arguments = NULL,

	.cancel_return_screen = &screen_menu,
	.cancel_return_screen_arguments = &main_menu_return_same_selection
};

static void sms_write_enter(struct nokia_ui *ui, void* arguments)
{
	struct screen_sms_write_arguments* args = arguments;
	assert(arguments);

	memset(phone_book_browse_name, 0, sizeof(phone_book_browse_name));
	memset(phone_book_browse_number, 0, sizeof(phone_book_browse_number));
	memset(text, 0, sizeof(text));
	num_sms = 0;
	text_len = 0;

	if(args->phone_number)
		strncpy(phone_book_browse_number, args->phone_number, sizeof(phone_book_browse_number)-1);

	phone_book_browse_id = -1;

	screen_set_current(ui, &screen_sms_write_show_editor, NULL);
}

struct screen screen_sms_write = {
	.enter_callback = sms_write_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = screen_draw_null,
	.handle_input = screen_handle_input_null,
	.handle_baseband = screen_handle_baseband_null,
};

struct screen_sms_write_arguments screen_sms_write_arguments_default = {
	.phone_number = NULL
};




static void sms_write_show_editor_enter(struct nokia_ui *ui, void* arguments)
{
	screen_set_current(ui, &screen_text_editor, &(struct screen_text_editor_arguments){
		.max_input_len = sizeof(text) - 1,
		.input_buffer = text,
		.entered_text_len = &text_len,
		.sms_enable_multipart = true,

		.button_text = "Options",
		.success_return_screen = &screen_menu,
		.success_return_screen_arguments = &sms_write_options_menu_arguments,

		.cancel_return_screen = &screen_menu,
		.cancel_return_screen_arguments = &main_menu_return_same_selection
	});
}

struct screen screen_sms_write_show_editor = {
	.enter_callback = sms_write_show_editor_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = screen_draw_null,
	.handle_input = screen_handle_input_null,
	.handle_baseband = screen_handle_baseband_null,
};




static void sms_write_enter_number_enter(struct nokia_ui *ui, void* arguments)
{
	screen_set_current(ui, &screen_input_dialog, &number_input_dialog_arguments);
}

struct screen screen_sms_write_enter_number = {
	.enter_callback = sms_write_enter_number_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = screen_draw_null,
	.handle_input = screen_handle_input_null,
	.handle_baseband = screen_handle_baseband_null,
};




static void sms_write_send_progress_event_cb(struct nokia_ui *ui, int event, void* data1, void* data2)
{
	struct ui_event_send_sms_done_params *params = data1;

	switch(event) {
		case UI_EVENT_SMS_SEND_DONE:
			if(params->cause == 0)
				screen_set_current(ui, &screen_info, &(struct screen_info_arguments){
					.text_line1 = "Message",
					.text_line2 = "sending",
					.text_line3 = "successful",
					.animation = &sms_anim,
					.return_screen = &screen_menu,
					.return_screen_arguments = &main_menu_return_same_selection
				});
			else
				screen_set_current(ui, &screen_info, &(struct screen_info_arguments){
					.text_line1 = "Message",
					.text_line2 = "sending",
					.text_line3 = "failed",
					.animation = &error_anim,
					.return_screen = &screen_menu,
					.return_screen_arguments = &sms_write_options_menu_arguments
				});
			break;
	}
}

static void sms_write_send_sms_enter(struct nokia_ui *ui, void* arguments)
{
	sms_send(ui, phone_book_browse_number, text);

	screen_set_current(ui, &screen_progress, &(struct screen_progress_arguments){
		.text_line1 = "Sending message",
		.text_line2 = "",
		.text_button = NULL,

		.button_screen = NULL,
		.button_screen_arguments = NULL,

		.handle_baseband_cb = sms_write_send_progress_event_cb,
	});
}

struct screen screen_sms_write_send_sms = {
	.enter_callback = sms_write_send_sms_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = screen_draw_null,
	.handle_input = screen_handle_input_null,
	.handle_baseband = screen_handle_baseband_null,
};




static void sms_write_choose_address_enter(struct nokia_ui *ui, void* arguments)
{
	screen_set_current(ui, &screen_phone_book_browser, &(struct screen_phone_book_browser_arguments){
		.selected_entry_id = &phone_book_browse_id,

		.max_name_len = sizeof(phone_book_browse_name) - 1,
		.name_buffer = phone_book_browse_name,

		.max_number_len = sizeof(phone_book_browse_number) - 1,
		.number_buffer = phone_book_browse_number,

		.button_text = "OK",

		.success_return_screen = &screen_sms_write_enter_number,
		.success_return_screen_arguments = NULL,

		.cancel_return_screen = &screen_sms_write_enter_number,
		.cancel_return_screen_arguments = NULL
	});
}

struct screen screen_sms_write_choose_address = {
	.enter_callback = sms_write_choose_address_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = screen_draw_null,
	.handle_input = screen_handle_input_null,
	.handle_baseband = screen_handle_baseband_null,
};




static int selected_template_id;
static int selected_template_index;

static void sms_write_insert_template_enter(struct nokia_ui *ui, void* arguments)
{
	if(db_get_template_entry_count() == 0)
	{
		screen_set_current(ui, &screen_info, &(struct screen_info_arguments){
			.text_line1 = "No",
			.text_line2 = "templates",
			.text_line3 = "",
			.animation = &info_anim,
			.return_screen = &screen_menu,
			.return_screen_arguments = &sms_write_options_menu_arguments
		});

		return;
	}

	screen_set_current(ui, &screen_template_list, &(struct screen_template_list_arguments) {
		.button_text = "Insert",
		.selected_template_id = &selected_template_id,
		.selected_template_index = &selected_template_index,

		.button_return_screen = &screen_sms_write_insert_selected_template,
		.button_return_screen_arguments = NULL,

		.return_screen = &screen_menu,
		.return_screen_arguments = &sms_write_options_menu_arguments
	});
}

struct screen screen_sms_write_insert_template = {
	.enter_callback = sms_write_insert_template_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = screen_draw_null,
	.handle_input = screen_handle_input_null,
	.handle_baseband = screen_handle_baseband_null,
};




static void sms_write_insert_selected_template_enter(struct nokia_ui *ui, void* arguments)
{
	uint8_t template_text[161];
	size_t template_len = 0;
	db_get_template(selected_template_index, NULL, template_text, &template_len);

	strncat(text+text_len, template_text, SMS_MAX_LEN-text_len);
	text_len = strlen(text);

	screen_set_current(ui, &screen_sms_write_show_editor, NULL);
}

struct screen screen_sms_write_insert_selected_template = {
	.enter_callback = sms_write_insert_selected_template_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = screen_draw_null,
	.handle_input = screen_handle_input_null,
	.handle_baseband = screen_handle_baseband_null,
};




struct menu sms_write_options_menu;

struct menu sms_send_screen = {
	.type = MENU_TYPE_SPECIAL,
	.num_entries = 0,
	.parent_menu = &sms_write_options_menu,
	.special_screen = &screen_sms_write_enter_number,
	.entries = NULL
};

struct menu sms_insert_template_screen = {
	.type = MENU_TYPE_SPECIAL,
	.num_entries = 0,
	.parent_menu = &sms_write_options_menu,
	.special_screen = &screen_sms_write_insert_template,
	.entries = NULL
};

struct menu_entry sms_write_options_menu_entries[] = {
	{
		.animation = NULL,
		.name = "Send",
		.sub_menu = &sms_send_screen
	},
	{
		.animation = NULL,
		.name = "Insert template",
		.sub_menu = &sms_insert_template_screen
	}
};

struct menu sms_write_options_menu = {
	.type = MENU_TYPE_SUB,
	.num_entries = 2,
	.parent_menu = NULL,
	.entries = sms_write_options_menu_entries,
	.selected_entry = 0,
	.submenu_current_top_item = 0
};

struct menu_instance sms_write_options_menu_instance = {
	.menu = &sms_write_options_menu,
	.return_screen = &screen_sms_write_show_editor,
	.return_screen_arguments = NULL
};


static struct screen_menu_arguments sms_write_options_menu_arguments = {
	.menu_instance = &sms_write_options_menu_instance,
	.item_to_select = 0
};
