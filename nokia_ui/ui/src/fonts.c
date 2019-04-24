#include "fonts.h"
#include "display.h"

void fonts_draw_text(struct nokia_ui* ui, const struct image* font[], const char* text, int x, int y)
{
	int curX = x;

	while(*text)
	{
		display_blit_image(ui, curX, y, font[(uint8_t)*text]);
		curX += font[(uint8_t)*text]->width;
		text++;
	}
}

void fonts_draw_int(struct nokia_ui* ui, const struct image* font[], int number, int x, int y)
{
	char buf[21] = {0};
	snprintf(buf, sizeof(buf), "%d", number);
	fonts_draw_text(ui, font, buf, x, y);
}

int fonts_get_pixel_width(const struct image* font[], const char* text)
{
	int cur_width = 0;

	while(*text)
	{
		cur_width += font[(uint8_t)*text]->width;
		text++;
	}

	return cur_width;
}

int fonts_get_glyph_pixel_width(const struct image* font[], const char chr)
{
	return font[(uint8_t)chr]->width;
}

int fonts_get_pixel_height(const struct image* font[])
{
	return font[0]->height;
}
