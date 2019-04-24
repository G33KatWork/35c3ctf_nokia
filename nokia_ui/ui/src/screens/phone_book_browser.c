#include "screens/phone_book_browser.h"

#include <assert.h>
#include <string.h>

#include "wordwrap.h"
#include "display.h"
#include "keyboard.h"
#include "fonts.h"
#include "db.h"

#include "screens/info.h"

#include "generated/font_large_bold.h"
#include "generated/font_small_bold.h"
#include "generated/nokia_images.h"

static _Bool phone_book_browse_name_wrap_callback(struct nokia_ui *ui, char *line, uint16_t count, void *state);

static struct screen_phone_book_browser_arguments browser_arguments;

static int phone_book_num_entries = 0;
static int phone_book_current_entry = 0;

static void phone_book_browser_enter(struct nokia_ui *ui, void* arguments)
{
	assert(arguments);
	memcpy(&browser_arguments, arguments, sizeof(struct screen_phone_book_browser_arguments));

	phone_book_num_entries = db_get_phone_book_entry_count();
	if(!browser_arguments.keep_old_selected_entry_id)
	{
		phone_book_current_entry = 0;
		*browser_arguments.selected_entry_id = -1;
	}

	if(phone_book_num_entries > 0)
		db_get_phone_book_entry(phone_book_current_entry, browser_arguments.selected_entry_id, browser_arguments.name_buffer, browser_arguments.number_buffer, browser_arguments.max_name_len, browser_arguments.max_number_len);
	else
		screen_set_current(ui, &screen_info, &(struct screen_info_arguments){
			.text_line1 = "No phone",
			.text_line2 = "numbers",
			.text_line3 = "",
			.animation = &info_anim,
			.return_screen = browser_arguments.cancel_return_screen,
			.return_screen_arguments = browser_arguments.cancel_return_screen_arguments
		});
}

static void phone_book_browser_draw(struct nokia_ui *ui)
{
	display_blit_image(ui, 0, 0, &book);

	int curLine = 0;
	wordwrap(ui, font_3310_large_bold, DISPLAY_WIDTH-4, browser_arguments.name_buffer, phone_book_browse_name_wrap_callback, &curLine);

	int x = DISPLAY_WIDTH/2 - fonts_get_pixel_width(font_3310_small_bold, browser_arguments.button_text) / 2;
	int y = DISPLAY_HEIGHT - fonts_get_pixel_height(font_3310_small_bold);
	fonts_draw_text(ui, font_3310_small_bold, browser_arguments.button_text, x, y);
}

static void phone_book_browser_handle_input(struct nokia_ui *ui, int keys)
{
	switch(keys)
	{
		case NOKIA_KEY_ENTER:
			screen_set_current(ui, browser_arguments.success_return_screen, browser_arguments.success_return_screen_arguments);
			break;

		case NOKIA_KEY_ESC:
			screen_set_current(ui, browser_arguments.cancel_return_screen, browser_arguments.cancel_return_screen_arguments);
			break;

		case NOKIA_KEY_DOWN:
			phone_book_current_entry++;
			if(phone_book_current_entry >= phone_book_num_entries)
				phone_book_current_entry = 0;

			db_get_phone_book_entry(phone_book_current_entry, browser_arguments.selected_entry_id, browser_arguments.name_buffer, browser_arguments.number_buffer, browser_arguments.max_name_len, browser_arguments.max_number_len);
			break;

		case NOKIA_KEY_UP:
			phone_book_current_entry--;
			if(phone_book_current_entry < 0)
				phone_book_current_entry = phone_book_num_entries-1;

			db_get_phone_book_entry(phone_book_current_entry, browser_arguments.selected_entry_id, browser_arguments.name_buffer, browser_arguments.number_buffer, browser_arguments.max_name_len, browser_arguments.max_number_len);
			break;
	}
}

static _Bool phone_book_browse_name_wrap_callback(struct nokia_ui *ui, char *line, uint16_t count, void *state)
{
	int* curLine = (int*)state;

	char* tmpstr = alloca(sizeof(char)*count + 1);
	memset(tmpstr, 0, sizeof(char)*count + 1);
	strncpy(tmpstr, line, count);

	fonts_draw_text(ui, font_3310_large_bold, tmpstr, 2, 8 + (*curLine) * (fonts_get_pixel_height(font_3310_small_bold)+6));

	(*curLine)++;

	return true;
}

struct screen screen_phone_book_browser = {
	.enter_callback = phone_book_browser_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = phone_book_browser_draw,
	.handle_input = phone_book_browser_handle_input,
	.handle_baseband = screen_handle_baseband_null,
};
