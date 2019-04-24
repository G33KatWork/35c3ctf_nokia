#include "wordwrap.h"

static _Bool is_wrap_space(char c)
{
	return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '-';
}

void wordwrap(struct nokia_ui *ui, const struct image* font[], int16_t width, char* text, line_callback_t callback, void *state)
{
	char* linestart;

	/* Current line width and character count */
	int16_t lw_cur = 0, cc_cur = 0;

	/* Previous wrap point */
	int16_t cc_prev;
	char* ls_prev;

	linestart = text;

	while (*text)
	{
		cc_prev = 0;
		ls_prev = text;

		while (*text)
		{
			char c;
			int16_t new_width;
			char* tmp;

			tmp = text;
			c = *text;
			text++;
			new_width = lw_cur + fonts_get_glyph_pixel_width(font, c);

			if (c == '\n')
			{
				cc_prev = cc_cur + 1;
				ls_prev = text;
				break;
			}

			if (new_width > width)
			{
				text = tmp;
				break;
			}

			cc_cur++;
			lw_cur = new_width;

			if (is_wrap_space(c))
			{
				cc_prev = cc_cur;
				ls_prev = text;
			}
		}

		/* Handle unbreakable words */
		if (cc_prev == 0)
		{
			cc_prev = cc_cur;
			ls_prev = text;
		}

		if (!callback(ui, linestart, cc_prev, state))
			return;

		linestart = ls_prev;
		text = linestart;
		lw_cur = 0;
		cc_cur = 0;
	}
}
