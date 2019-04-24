#include "screens/sms_inbox.h"

#include "db.h"
#include "animations.h"

#include "screens/info.h"

#include "screens/menu.h"
#include "screens/sms_inbox_list.h"
#include "menus/main_menu.h"

#include <string.h>

static void sms_inbox_screen_enter(struct nokia_ui *ui, void* arguments)
{
	int message_count = db_get_message_entry_count();

	if(message_count == 0)
	{
		screen_set_current(ui, &screen_info, &(struct screen_info_arguments){
			.text_line1 = "No",
			.text_line2 = "messages",
			.text_line3 = "in Inbox",
			.animation = &success_anim,
			.return_screen = &screen_menu,
			.return_screen_arguments = &main_menu_return_same_selection
		});

		return;
	}

	screen_set_current(ui, &screen_sms_inbox_list, &(struct screen_sms_inbox_list_arguments) {
		.button_text = "View",
		.button_return_screen = &screen_menu,
		.button_return_screen_arguments = &main_menu_return_same_selection,
		.return_screen = &screen_menu,
		.return_screen_arguments = &main_menu_return_same_selection
	});
}

struct screen screen_sms_inbox = {
	.enter_callback = sms_inbox_screen_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = screen_draw_null,
	.handle_input = screen_handle_input_null,
	.handle_baseband = screen_handle_baseband_null,
};
