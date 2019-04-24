#include "screens/sms_write.h"

#include "db.h"
#include "screen.h"
#include "animations.h"

#include "screens/menu.h"
#include "screens/text_editor.h"
#include "screens/info.h"

#include "menus/main_menu.h"

#include <stdlib.h>
#include <string.h>

struct screen screen_save_template;

static char template_text[161];
static int template_len;

static void template_create_enter(struct nokia_ui *ui, void* arguments)
{
	memset(template_text, 0, sizeof(template_text));
	template_len = 0;

	screen_set_current(ui, &screen_text_editor, &(struct screen_text_editor_arguments){
		.max_input_len = sizeof(template_text) - 1,
		.input_buffer = template_text,
		.entered_text_len = &template_len,
		.sms_enable_multipart = false,

		.button_text = "Save",
		.success_return_screen = &screen_save_template,
		.success_return_screen_arguments = NULL,

		.cancel_return_screen = &screen_menu,
		.cancel_return_screen_arguments = &main_menu_return_same_selection
	});
}

struct screen screen_template_create = {
	.enter_callback = template_create_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = screen_draw_null,
	.handle_input = screen_handle_input_null,
	.handle_baseband = screen_handle_baseband_null,
};




static void template_create_save_enter(struct nokia_ui *ui, void* arguments)
{
	db_add_template(template_text, template_len);

	screen_set_current(ui, &screen_info, &(struct screen_info_arguments){
		.text_line1 = "Saved",
		.text_line2 = "",
		.text_line3 = "",
		.animation = &success_anim,
		.return_screen = &screen_menu,
		.return_screen_arguments = &main_menu_return_same_selection
	});
}

struct screen screen_save_template = {
	.enter_callback = template_create_save_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = screen_draw_null,
	.handle_input = screen_handle_input_null,
	.handle_baseband = screen_handle_baseband_null,
};
