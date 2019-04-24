#include "screens/template_manage.h"

#include "screen.h"
#include "db.h"

#include "screens/template_list.h"
#include "screens/template_viewer.h"
#include "screens/info.h"
#include "screens/menu.h"

#include "menus/main_menu.h"

#include <stdlib.h>
#include <string.h>

struct screen screen_template_perform_deletion;
struct screen screen_template_open_viewer;

static int selected_template_id;
static int selected_template_index;
static char selected_template_text[161];

static void template_manage_enter(struct nokia_ui *ui, void* arguments)
{
	memset(selected_template_text, 0, sizeof(selected_template_text));

	if(db_get_template_entry_count() == 0)
	{
		screen_set_current(ui, &screen_info, &(struct screen_info_arguments) {
			.text_line1 = "No",
			.text_line2 = "templates",
			.text_line3 = "",
			.animation = &info_anim,
			.return_screen = &screen_menu,
			.return_screen_arguments = &main_menu_return_same_selection
		});

		return;
	}

	screen_set_current(ui, &screen_template_list, &(struct screen_template_list_arguments) {
		.button_text = "View",
		.selected_template_id = &selected_template_id,
		.selected_template_index = &selected_template_index,

		.button_return_screen = &screen_template_open_viewer,
		.button_return_screen_arguments = NULL,

		.return_screen = &screen_menu,
		.return_screen_arguments = &main_menu_return_same_selection
	});
}

struct screen screen_template_manage = {
	.enter_callback = template_manage_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = screen_draw_null,
	.handle_input = screen_handle_input_null,
	.handle_baseband = screen_handle_baseband_null,
};



static void template_open_viewer_enter(struct nokia_ui *ui, void* arguments)
{
	db_get_template(selected_template_index, NULL, selected_template_text, NULL);

	screen_set_current(ui, &screen_template_viewer, &(struct template_viewer_arguments) {
		.text = selected_template_text,
		.button_text = "Delete",
		.return_screen = &screen_template_manage,
		.return_screen_arguments = NULL,
		.button_screen = &screen_template_perform_deletion,
		.button_screen_arguments = NULL
	});
}

struct screen screen_template_open_viewer = {
	.enter_callback = template_open_viewer_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = screen_draw_null,
	.handle_input = screen_handle_input_null,
	.handle_baseband = screen_handle_baseband_null,
};




static void template_perform_deletion_enter(struct nokia_ui *ui, void* arguments)
{
	db_del_template(selected_template_id);

	screen_set_current(ui, &screen_info, &(struct screen_info_arguments){
		.text_line1 = "Memory",
		.text_line2 = "erased",
		.text_line3 = "",
		.animation = &erase_anim,
		.return_screen = &screen_template_manage,
		.return_screen_arguments = NULL
	});
}

struct screen screen_template_perform_deletion = {
	.enter_callback = template_perform_deletion_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = screen_draw_null,
	.handle_input = screen_handle_input_null,
	.handle_baseband = screen_handle_baseband_null,
};
