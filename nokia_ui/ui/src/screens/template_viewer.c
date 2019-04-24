#include "screens/template_viewer.h"

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

static _Bool template_viewer_count_lines(struct nokia_ui *ui, char *line, uint16_t count, void *state);
static _Bool template_viewer_add_line_callback(struct nokia_ui *ui, char *line, uint16_t count, void *state);


static int viewer_current_top_line = 0;
static int viewer_total_line_count = 0;

static struct template_viewer_arguments template_viewer_arguments;

static void template_viewer_enter(struct nokia_ui *ui, void* arguments)
{
	assert(arguments);
	memcpy(&template_viewer_arguments, arguments, sizeof(struct template_viewer_arguments));

	viewer_current_top_line = 0;
	viewer_total_line_count = 0;

	wordwrap(ui, font_3310_small_bold, DISPLAY_WIDTH, template_viewer_arguments.text, template_viewer_count_lines, &viewer_total_line_count);
}

static void template_viewer_draw(struct nokia_ui *ui)
{
	struct wordwrap_state state = {
		.curLine = 0,
		.totalLines = viewer_total_line_count
	};
	wordwrap(ui, font_3310_small_bold, DISPLAY_WIDTH, template_viewer_arguments.text, template_viewer_add_line_callback, &state);

	int btnx = DISPLAY_WIDTH/2 - fonts_get_pixel_width(font_3310_small_bold, template_viewer_arguments.button_text) / 2;
	int btny = DISPLAY_HEIGHT - fonts_get_pixel_height(font_3310_small_bold);
	fonts_draw_text(ui, font_3310_small_bold, template_viewer_arguments.button_text, btnx, btny);
}

static void template_viewer_handle_input(struct nokia_ui *ui, int keys)
{
	int displayed_text_lines = viewer_total_line_count > 5 ? 5 : viewer_total_line_count;

	switch(keys)
	{
		case NOKIA_KEY_ENTER:
			screen_set_current(ui, template_viewer_arguments.button_screen, template_viewer_arguments.button_screen_arguments);
			break;

		case NOKIA_KEY_ESC:
			screen_set_current(ui, template_viewer_arguments.return_screen, template_viewer_arguments.return_screen_arguments);
			break;

		case NOKIA_KEY_DOWN:
			if(viewer_current_top_line + displayed_text_lines < viewer_total_line_count)
				viewer_current_top_line++;
			break;

		case NOKIA_KEY_UP:
			if(viewer_current_top_line > 0)
				viewer_current_top_line--;
			break;
	}
}

static _Bool template_viewer_count_lines(struct nokia_ui *ui, char *line, uint16_t count, void *state)
{
	int *linecount = (int*)state;
	(*linecount)++;
	return true;
}

static _Bool template_viewer_add_line_callback(struct nokia_ui *ui, char *line, uint16_t count, void *state)
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

struct screen screen_template_viewer = {
	.enter_callback = template_viewer_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = template_viewer_draw,
	.handle_input = template_viewer_handle_input,
	.handle_baseband = screen_handle_baseband_null,
};
