#ifndef _FONTS_H_
#define _FONTS_H_

#include "nokia_ui.h"
#include "images.h"

void fonts_draw_text(struct nokia_ui* ui, const struct image* font[], const char* text, int x, int y);
void fonts_draw_int(struct nokia_ui* ui, const struct image* font[], int number, int x, int y);
int fonts_get_pixel_width(const struct image* font[], const char* text);
int fonts_get_glyph_pixel_width(const struct image* font[], const char chr);
int fonts_get_pixel_height(const struct image* font[]);

#endif
