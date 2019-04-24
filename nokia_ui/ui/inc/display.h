#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "nokia_ui.h"
#include "images.h"

#define DISPLAY_WIDTH 84
#define DISPLAY_HEIGHT 48

void display_draw_line(struct nokia_ui* ui, int startx, int starty, int endx, int endy);
void display_draw_line_clear(struct nokia_ui* ui, int startx, int starty, int endx, int endy);
void display_draw_line_set_clear(struct nokia_ui* ui, int startx, int starty, int endx, int endy, int set);
void display_draw_rectangle(struct nokia_ui* ui, int startx, int starty, int endx, int endy, int set);
void display_draw_rectangle_fill(struct nokia_ui* ui, int startx, int starty, int endx, int endy, int set);
void display_invert_box(struct nokia_ui* ui, int startx, int starty, int endx, int endy);
void display_blit_image(struct nokia_ui* ui, int x, int y, const struct image* image);

#endif
