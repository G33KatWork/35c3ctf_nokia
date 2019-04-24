#include "display.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "ui.h"

void display_draw_line(struct nokia_ui* ui, int startx, int starty, int endx, int endy)
{
	display_draw_line_set_clear(ui, startx, starty, endx, endy, 1);
}

void display_draw_line_clear(struct nokia_ui* ui, int startx, int starty, int endx, int endy)
{
	display_draw_line_set_clear(ui, startx, starty, endx, endy, 0);
}

void display_draw_line_set_clear(struct nokia_ui* ui, int startx, int starty, int endx, int endy, int set)
{
	int t, distance;
	int xerr=0, yerr=0, delta_x, delta_y;
	int incx, incy;

	/* compute the distances in both directions */
	delta_x=endx-startx;
	delta_y=endy-starty;

	/* Compute the direction of the increment,
	an increment of 0 means either a horizontal or vertical
	line.
	*/
	if(delta_x>0) incx=1;
	else if(delta_x==0) incx=0;
	else incx=-1;

	if(delta_y>0) incy=1;
	else if(delta_y==0) incy=0;
	else incy=-1;

	/* determine which distance is greater */
	delta_x=abs(delta_x);
	delta_y=abs(delta_y);
	if(delta_x>delta_y) distance=delta_x;
	else distance=delta_y;

	/* draw the line */
	for(t=0; t<=distance+1; t++) {
		if(set)
			ui_display_set_pixel(ui, startx, starty);
		else
			ui_display_clear_pixel(ui, startx, starty);

		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance) {
			xerr-=distance;
			startx+=incx;
		}
		if(yerr>distance) {
			yerr-=distance;
			starty+=incy;
		}
	}
}

void display_draw_rectangle(struct nokia_ui* ui, int startx, int starty, int endx, int endy, int set)
{
	display_draw_line_set_clear(ui, startx, starty, endx, starty, set);
	display_draw_line_set_clear(ui, startx, starty, startx, endy, set);
	display_draw_line_set_clear(ui, startx, endy, endx, endy, set);
	display_draw_line_set_clear(ui, endx, starty, endx, endy, set);
}

void display_draw_rectangle_fill(struct nokia_ui* ui, int startx, int starty, int endx, int endy, int set)
{
	for(int y = starty; y < endy; y++)
		display_draw_line_set_clear(ui, startx, y, endx, y, set);
}

void display_invert_box(struct nokia_ui* ui, int startx, int starty, int endx, int endy)
{
	for(int curYpos = starty; curYpos < DISPLAY_HEIGHT && (curYpos-starty) < endy; curYpos++)
	{
		for(int curXpos = startx; curXpos < DISPLAY_WIDTH && (curXpos-startx) < endx; curXpos++)
		{
			ui_display_invert_pixel(ui, curXpos, curYpos);
		}
	}
}

void display_blit_image(struct nokia_ui* ui, int x, int y, const struct image* image)
{
	if(x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT)
		return;

	for(int curYpos = y; curYpos < DISPLAY_HEIGHT && (curYpos-y) < image->height; curYpos++)
	{
		for(int curXpos = x; curXpos < DISPLAY_WIDTH && (curXpos-x) < image->width; curXpos++)
		{
			uint16_t byteOffset = (curXpos-x + ((curYpos-y)*image->width)) / 8;
			//uint8_t bitOffset = (curXpos % 8);
			uint8_t bitOffset = ((curXpos-x) + ((curYpos-y)*image->width)) % 8;

			if(byteOffset >= image->datalen)
				return;

			if(image->data[byteOffset] & (1<<bitOffset))
				ui_display_set_pixel(ui, curXpos, curYpos);
			else
				ui_display_clear_pixel(ui, curXpos, curYpos);
		}
	}
}
