#include "screens/info.h"

#include <string.h>
#include <assert.h>

#include "display.h"
#include "keyboard.h"
#include "fonts.h"
#include "animations.h"
#include "screens/menu.h"

#include "generated/font_large_bold.h"
#include "generated/nokia_images.h"

static void info_screen_draw(struct nokia_ui *ui);

static struct screen_info_arguments info_arguments;

static void info_screen_enter(struct nokia_ui *ui, void* arguments)
{
	assert(arguments);
	memcpy(&info_arguments, arguments, sizeof(struct screen_info_arguments));

	animation_queue(0, info_arguments.animation);
	info_screen_draw(ui);	//Hack to avoid animation being drawn without text
}

static void info_screen_leave(struct nokia_ui *ui)
{
	animation_clear();
}

static void info_screen_draw(struct nokia_ui *ui)
{
	fonts_draw_text(ui, font_3310_large_bold, info_arguments.text_line1, 0, 0);
	fonts_draw_text(ui, font_3310_large_bold, info_arguments.text_line2, 0, fonts_get_pixel_height(font_3310_large_bold) + 2);
	fonts_draw_text(ui, font_3310_large_bold, info_arguments.text_line3, 0, (fonts_get_pixel_height(font_3310_large_bold) + 2) * 2);

	if(animation_isdone(0))
		screen_set_current(ui, info_arguments.return_screen, info_arguments.return_screen_arguments);
}

static void info_screen_handle_input(struct nokia_ui *ui, int keys)
{
	switch(keys)
	{
		case NOKIA_KEY_ENTER:
		case NOKIA_KEY_ESC:
			screen_set_current(ui, info_arguments.return_screen, info_arguments.return_screen_arguments);
			break;
	}
}

struct screen screen_info = {
	.enter_callback = info_screen_enter,
	.leave_callback = info_screen_leave,
	.draw_callback = info_screen_draw,
	.handle_input = info_screen_handle_input,
	.handle_baseband = screen_handle_baseband_null,
};
