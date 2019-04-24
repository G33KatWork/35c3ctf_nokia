#include "screens/input_dialog.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <alloca.h>

#include "display.h"
#include "keyboard.h"
#include "fonts.h"
#include "wordwrap.h"

#include "generated/nokia_images.h"
#include "generated/font_small_bold.h"

static struct screen_input_dialog_arguments input_dialog_arguments;

static _Bool input_dialog_add_line_callback(struct nokia_ui *ui, char *line, uint16_t count, void *state);

static void input_dialog_enter(struct nokia_ui *ui, void* arguments)
{
	assert(arguments);
	memcpy(&input_dialog_arguments, arguments, sizeof(struct screen_input_dialog_arguments));
}

static void input_dialog_draw(struct nokia_ui *ui)
{
	char buf[128] = {0};
	int oky;
	int okx;

	display_blit_image(ui, 0, 0, &pen);

	if(input_dialog_arguments.type == INPUT_DLG_TYPE_TEXT)
		display_blit_image(ui, 9, 0, &abc);
	else
		display_blit_image(ui, 9, 0, &_123);

	snprintf(buf, sizeof(buf), "%d", (int)(input_dialog_arguments.max_input_len - strlen(input_dialog_arguments.input_buffer)));
	fonts_draw_text(ui, font_3310_small_bold, buf, DISPLAY_WIDTH - fonts_get_pixel_width(font_3310_small_bold, buf), 0);

	fonts_draw_text(ui, font_3310_small_bold, input_dialog_arguments.prompt_text, 0, 8);

	display_draw_line(ui, 0, 16, DISPLAY_WIDTH-1, 16);
	display_draw_line(ui, 0, 16, 0, 34);
	display_draw_line(ui, DISPLAY_WIDTH-1, 16, DISPLAY_WIDTH-1, 34);
	display_draw_line(ui, 0, 34, DISPLAY_WIDTH-1, 34);
	int curLine = 0;
	wordwrap(ui, font_3310_small_bold, DISPLAY_WIDTH-4, input_dialog_arguments.input_buffer, input_dialog_add_line_callback, &curLine);

	if(strlen(input_dialog_arguments.input_buffer) > 0)
	{
		okx = DISPLAY_WIDTH/2 - fonts_get_pixel_width(font_3310_small_bold, input_dialog_arguments.button_text) / 2;
		oky = DISPLAY_HEIGHT - fonts_get_pixel_height(font_3310_small_bold);
		fonts_draw_text(ui, font_3310_small_bold, input_dialog_arguments.button_text, okx, oky);
	}
	else
	{
		okx = DISPLAY_WIDTH/2 - fonts_get_pixel_width(font_3310_small_bold, input_dialog_arguments.button_text_empty) / 2;
		oky = DISPLAY_HEIGHT - fonts_get_pixel_height(font_3310_small_bold);
		fonts_draw_text(ui, font_3310_small_bold, input_dialog_arguments.button_text_empty, okx, oky);
	}
}

static void input_dialog_handle_input(struct nokia_ui *ui, int keys)
{
	char chr;

	if(input_dialog_arguments.type == INPUT_DLG_TYPE_TEXT)
	{
		switch(keys)
		{
			case 'a' ... 'z':
			case 'A' ... 'Z':
			case '0' ... '9':
			case ' ' ... '/':
			case ':' ... '@':
			case '_':
				if(strlen(input_dialog_arguments.input_buffer) < input_dialog_arguments.max_input_len)
				{
					chr = (char)keys;
					input_dialog_arguments.input_buffer[strlen(input_dialog_arguments.input_buffer)] = chr;
				}
				break;
		}
	}
	else
	{
		switch(keys)
		{
			case '0' ... '9':
				if(strlen(input_dialog_arguments.input_buffer) < input_dialog_arguments.max_input_len)
				{
					chr = (char)keys;
					input_dialog_arguments.input_buffer[strlen(input_dialog_arguments.input_buffer)] = chr;
				}
				break;
		}
	}

	switch(keys)
	{
		case NOKIA_KEY_BACKSPACE:
			if(strlen(input_dialog_arguments.input_buffer) > 0)
				input_dialog_arguments.input_buffer[strlen(input_dialog_arguments.input_buffer)-1] = 0;
			break;

		case NOKIA_KEY_ENTER:
			if(strlen(input_dialog_arguments.input_buffer) > 0)
				screen_set_current(ui, input_dialog_arguments.success_return_screen, input_dialog_arguments.success_return_screen_arguments);
			else
				screen_set_current(ui, input_dialog_arguments.empty_return_screen, input_dialog_arguments.empty_return_screen_arguments);
			break;

		case NOKIA_KEY_ESC:
			screen_set_current(ui, input_dialog_arguments.cancel_return_screen, input_dialog_arguments.cancel_return_screen_arguments);
			break;
	}
}

static _Bool input_dialog_add_line_callback(struct nokia_ui *ui, char *line, uint16_t count, void *state)
{
	int* curLine = (int*)state;

	char* tmpstr = alloca(sizeof(char)*count + 1);
	memset(tmpstr, 0, sizeof(char)*count + 1);
	strncpy(tmpstr, line, count);

	fonts_draw_text(ui, font_3310_small_bold, tmpstr, 2, 18 + (*curLine) * fonts_get_pixel_height(font_3310_small_bold));

	(*curLine)++;

	return true;
}

struct screen screen_input_dialog = {
	.enter_callback = input_dialog_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = input_dialog_draw,
	.handle_input = input_dialog_handle_input,
	.handle_baseband = screen_handle_baseband_null,
};
