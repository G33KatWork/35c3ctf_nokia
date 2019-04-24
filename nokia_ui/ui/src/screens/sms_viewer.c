#include "screens/sms_viewer.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#include "display.h"
#include "keyboard.h"
#include "fonts.h"
#include "wordwrap.h"

#include "generated/font_small_bold.h"

struct wordwrap_state {
	int curLine;
	int totalLines;
};

static _Bool sms_viewer_count_lines(struct nokia_ui *ui, char *line, uint16_t count, void *state);
static _Bool sms_viewer_add_line_callback(struct nokia_ui *ui, char *line, uint16_t count, void *state);
static _Bool sms_viewer_simple_add_line_callback(struct nokia_ui *ui, char *line, uint16_t count, void *state);

static int viewer_current_top_line = 0;
static int viewer_total_line_count = 0;

struct sms_viewer_arguments viewer_arguments;

static void sms_viewer_enter(struct nokia_ui *ui, void* arguments)
{
	assert(arguments);
	memcpy(&viewer_arguments, arguments, sizeof(struct sms_viewer_arguments));

	viewer_current_top_line = 0;
	viewer_total_line_count = 0;

	wordwrap(ui, font_3310_small_bold, DISPLAY_WIDTH, viewer_arguments.text, sms_viewer_count_lines, &viewer_total_line_count);
}

static void sms_viewer_draw(struct nokia_ui *ui)
{
	char timestr[64] = {0};
	int displayed_text_lines = viewer_total_line_count > 5 ? 5 : viewer_total_line_count;

	if(viewer_current_top_line + displayed_text_lines - 1 < viewer_total_line_count)
	{
		struct wordwrap_state state = {
			.curLine = 0,
			.totalLines = viewer_total_line_count
		};
		wordwrap(ui, font_3310_small_bold, DISPLAY_WIDTH, viewer_arguments.text, sms_viewer_add_line_callback, &state);
	}
	else if(viewer_current_top_line + displayed_text_lines - 2 < viewer_total_line_count)
	{
		fonts_draw_text(ui, font_3310_small_bold, "Sender:", 0, 0);
		int curLine = 0;
		if(viewer_arguments.sender_name[0] != '\0')
			wordwrap(ui, font_3310_small_bold, DISPLAY_WIDTH, viewer_arguments.sender_name, sms_viewer_simple_add_line_callback, &curLine);
		wordwrap(ui, font_3310_small_bold, DISPLAY_WIDTH, viewer_arguments.number, sms_viewer_simple_add_line_callback, &curLine);
	}
	else
	{
		fonts_draw_text(ui, font_3310_small_bold, "Sent:", 0, 0);
		int curLine = 0;

		time_t t = *viewer_arguments.date;
		strftime(timestr, sizeof(timestr), "%d-%h-%Y %T", localtime(&t));
		wordwrap(ui, font_3310_small_bold, DISPLAY_WIDTH, timestr, sms_viewer_simple_add_line_callback, &curLine);
	}

	int btnx = DISPLAY_WIDTH/2 - fonts_get_pixel_width(font_3310_small_bold, viewer_arguments.button_text) / 2;
	int btny = DISPLAY_HEIGHT - fonts_get_pixel_height(font_3310_small_bold);
	fonts_draw_text(ui, font_3310_small_bold, viewer_arguments.button_text, btnx, btny);
}

static void sms_viewer_handle_input(struct nokia_ui *ui, int keys)
{
	int displayed_text_lines = viewer_total_line_count > 5 ? 5 : viewer_total_line_count;

	switch(keys)
	{
		case NOKIA_KEY_ENTER:
			screen_set_current(ui, viewer_arguments.button_screen, viewer_arguments.button_screen_arguments);
			break;

		case NOKIA_KEY_ESC:
			screen_set_current(ui, viewer_arguments.return_screen, viewer_arguments.return_screen_arguments);
			break;

		case NOKIA_KEY_DOWN:
			if(viewer_current_top_line + displayed_text_lines - 2 < viewer_total_line_count)
				viewer_current_top_line++;
			break;

		case NOKIA_KEY_UP:
			if(viewer_current_top_line > 0)
				viewer_current_top_line--;
			break;
	}
}

static _Bool sms_viewer_count_lines(struct nokia_ui *ui, char *line, uint16_t count, void *state)
{
	int *linecount = (int*)state;
	(*linecount)++;
	return true;
}

static _Bool sms_viewer_add_line_callback(struct nokia_ui *ui, char *line, uint16_t count, void *state)
{
	struct wordwrap_state* ww_state = state;

	if(ww_state->curLine >= viewer_current_top_line && ww_state->curLine - viewer_current_top_line < 5)
	{
		char* tmpstr = alloca(sizeof(char)*count + 1);
		memset(tmpstr, 0, sizeof(char)*count + 1);
		strncpy(tmpstr, line, count);

		fonts_draw_text(ui, font_3310_small_bold, tmpstr, 0, (ww_state->curLine - viewer_current_top_line) * fonts_get_pixel_height(font_3310_small_bold));
	}

	ww_state->curLine++;

	return true;
}

static _Bool sms_viewer_simple_add_line_callback(struct nokia_ui *ui, char *line, uint16_t count, void *state)
{
	int* curLine = (int*)state;

	char* tmpstr = alloca(sizeof(char)*count + 1);
	memset(tmpstr, 0, sizeof(char)*count + 1);
	strncpy(tmpstr, line, count);

	fonts_draw_text(ui, font_3310_small_bold, tmpstr, 0, fonts_get_pixel_height(font_3310_small_bold) + (*curLine) * fonts_get_pixel_height(font_3310_small_bold));

	(*curLine)++;

	return true;
}

struct screen screen_sms_viewer = {
	.enter_callback = sms_viewer_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = sms_viewer_draw,
	.handle_input = sms_viewer_handle_input,
	.handle_baseband = screen_handle_baseband_null,
};
