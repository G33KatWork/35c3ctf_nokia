#include "screens/text_editor.h"

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

struct wordwrap_state {
	int curLine;
	int totalLines;
};

static _Bool text_editor_count_lines(struct nokia_ui *ui, char *line, uint16_t count, void *state);
static _Bool text_editor_line_callback(struct nokia_ui *ui, char *line, uint16_t count, void *state);

static struct screen_text_editor_arguments text_editor_arguments;

static void text_editor_enter(struct nokia_ui *ui, void* arguments)
{
	assert(arguments);
	memcpy(&text_editor_arguments, arguments, sizeof(struct screen_text_editor_arguments));
}

static void text_editor_draw(struct nokia_ui *ui)
{
	char buf[128] = {0};

	display_blit_image(ui, 0, 0, &pen);
	display_blit_image(ui, 9, 0, &abc);

	if(text_editor_arguments.sms_enable_multipart) {
		int num_parts = (*text_editor_arguments.entered_text_len) <= 160 ? 1 : ((*text_editor_arguments.entered_text_len - 1)/153) + 1;
		snprintf(buf, sizeof(buf), "%d/%d", (int)(text_editor_arguments.max_input_len - (*text_editor_arguments.entered_text_len)), num_parts);
	} else
		snprintf(buf, sizeof(buf), "%d", (int)(text_editor_arguments.max_input_len - (*text_editor_arguments.entered_text_len)));

	fonts_draw_text(ui, font_3310_small_bold, buf, DISPLAY_WIDTH - fonts_get_pixel_width(font_3310_small_bold, buf), 0);

	int menux = DISPLAY_WIDTH/2 - fonts_get_pixel_width(font_3310_small_bold, text_editor_arguments.button_text) / 2;
	int menuy = DISPLAY_HEIGHT - fonts_get_pixel_height(font_3310_small_bold);
	fonts_draw_text(ui, font_3310_small_bold, text_editor_arguments.button_text, menux, menuy);

	int linecount = 0;
	wordwrap(ui, font_3310_small_bold, DISPLAY_WIDTH, text_editor_arguments.input_buffer, text_editor_count_lines, &linecount);

	struct wordwrap_state state = {
		.curLine = 0,
		.totalLines = linecount
	};
	wordwrap(ui, font_3310_small_bold, DISPLAY_WIDTH, text_editor_arguments.input_buffer, text_editor_line_callback, &state);
}

static void text_editor_handle_input(struct nokia_ui *ui, int keys)
{
	char chr;

	switch(keys)
	{
		case 'a' ... 'z':
		case 'A' ... 'Z':
		case '0' ... '9':
		case ' ' ... '/':
		case ':' ... '@':
		case '_':
			if((*text_editor_arguments.entered_text_len) < text_editor_arguments.max_input_len)
			{
				chr = (char)keys;
				text_editor_arguments.input_buffer[(*text_editor_arguments.entered_text_len)] = chr;
				(*text_editor_arguments.entered_text_len)++;
			}
			break;

		case NOKIA_KEY_BACKSPACE:
			if((*text_editor_arguments.entered_text_len) > 0)
			{
				text_editor_arguments.input_buffer[(*text_editor_arguments.entered_text_len)-1] = 0;
				(*text_editor_arguments.entered_text_len)--;
			}
			break;

		case NOKIA_KEY_ENTER:
			screen_set_current(ui, text_editor_arguments.success_return_screen, text_editor_arguments.success_return_screen_arguments);
			break;

		case NOKIA_KEY_ESC:
			screen_set_current(ui, text_editor_arguments.cancel_return_screen, text_editor_arguments.cancel_return_screen_arguments);
			break;
	}
}


static _Bool text_editor_count_lines(struct nokia_ui *ui, char *line, uint16_t count, void *state)
{
	int *linecount = (int*)state;
	(*linecount)++;
	return true;
}

static _Bool text_editor_line_callback(struct nokia_ui *ui, char *line, uint16_t count, void *state)
{
	struct wordwrap_state* ww_state = state;
	int line_on_display = (ww_state->totalLines - 1) - ww_state->curLine;

	if(line_on_display < 4)
	{
		char* tmpstr = alloca(sizeof(char)*count + 1);
		memset(tmpstr, 0, sizeof(char)*count + 1);
		strncpy(tmpstr, line, count);

		if(ww_state->totalLines > 4)
			fonts_draw_text(ui, font_3310_small_bold, tmpstr, 0, 8 + (3 - line_on_display) * fonts_get_pixel_height(font_3310_small_bold));
		else
			fonts_draw_text(ui, font_3310_small_bold, tmpstr, 0, 8 + (ww_state->totalLines - 1 - line_on_display) * fonts_get_pixel_height(font_3310_small_bold));
	}

	ww_state->curLine++;

	return true;
}

struct screen screen_text_editor = {
	.enter_callback = text_editor_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = text_editor_draw,
	.handle_input = text_editor_handle_input,
	.handle_baseband = screen_handle_baseband_null,
};
