#include "screens/template_list.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "db.h"
#include "display.h"
#include "keyboard.h"
#include "fonts.h"

#include "generated/font_small_bold.h"
#include "generated/nokia_images.h"

struct template {
	int id;
	char* text;
};

static struct screen_template_list_arguments template_list_arguments;

static int template_count;
static struct template* templates;
static int current_top_item;
static int selected_item;

static void template_list_draw_scrollbar(struct nokia_ui *ui);
static void template_list_select_entry(int newselection);
static void template_list_select(struct nokia_ui *ui);

static void template_list_enter(struct nokia_ui *ui, void* arguments)
{
	assert(arguments);
	memcpy(&template_list_arguments, arguments, sizeof(template_list_arguments));

	current_top_item = 0;
	selected_item = 0;

	template_count = db_get_template_entry_count();

	templates = malloc(template_count * sizeof(struct template));
	for(int i = 0; i < template_count; i++)
	{
		templates[i].text = malloc(161);
		memset(templates[i].text, 0, 161);

		db_get_template(i, &templates[i].id, templates[i].text, NULL);
	}
}

static void template_list_leave(struct nokia_ui *ui)
{
	for(int i = 0; i < template_count; i++)
	{
		free(templates[i].text);
	}

	free(templates);
}

static void template_list_draw(struct nokia_ui *ui)
{
	int xpos = 0, ypos = 0;
	char buf[20] = {0};

	snprintf(buf, sizeof(buf), "%d", selected_item+1);
	xpos = DISPLAY_WIDTH - fonts_get_pixel_width(font_3310_small_bold, buf);
	fonts_draw_text(ui, font_3310_small_bold, buf, xpos, 0);

	xpos = 0;
	ypos = 9;

	for(int i = current_top_item; i < current_top_item+3 && i < template_count; i++)
	{
		fonts_draw_text(ui, font_3310_small_bold, templates[i].text, xpos+1, ypos);
		if(i == selected_item)
			display_invert_box(ui, 0, ypos-1, DISPLAY_WIDTH-5, fonts_get_pixel_height(font_3310_small_bold)+1);

		ypos += fonts_get_pixel_height(font_3310_small_bold) + 2;
	}

	xpos = DISPLAY_WIDTH/2 - fonts_get_pixel_width(font_3310_small_bold, template_list_arguments.button_text)/2;
	ypos = DISPLAY_HEIGHT - fonts_get_pixel_height(font_3310_small_bold);
	fonts_draw_text(ui, font_3310_small_bold, template_list_arguments.button_text, xpos, ypos);

	template_list_draw_scrollbar(ui);
}

static void template_list_handle_input(struct nokia_ui *ui, int keys)
{
	switch(keys)
	{
		case NOKIA_KEY_ESC:
			screen_set_current(ui, template_list_arguments.return_screen, template_list_arguments.return_screen_arguments);
			break;

		case NOKIA_KEY_ENTER:
			template_list_select(ui);
			break;

		case NOKIA_KEY_DOWN:
			template_list_select_entry(selected_item+1);
			break;

		case NOKIA_KEY_UP:
			template_list_select_entry(selected_item-1);
			break;
	}
}

struct screen screen_template_list = {
	.enter_callback = template_list_enter,
	.leave_callback = template_list_leave,
	.draw_callback = template_list_draw,
	.handle_input = template_list_handle_input,
	.handle_baseband = screen_handle_baseband_null,
};


static void template_list_draw_scrollbar(struct nokia_ui *ui)
{
	int xpos = 0, ypos = 0;

	if(template_count > 0)
	{
		ypos = 11;

		if(template_count > 1)
			ypos += (22 / (template_count-1)) * selected_item;

		ypos -= menu_bommel.height/2;
		xpos = DISPLAY_WIDTH - menu_bommel.width;
		display_blit_image(ui, xpos, ypos, &menu_bommel);

		int len = ypos-8;
		xpos = DISPLAY_WIDTH-3;
		int ypos2 = 8;
		display_draw_line(ui, xpos, ypos2, xpos, ypos2+len);

		ypos2 = 35;
		display_draw_line(ui, xpos, ypos+6, xpos, ypos2);
	}
}


static void template_list_select_entry(int newselection)
{
	//going up
	if(newselection < current_top_item)
	{
		if(newselection < 0)
		{
			newselection = template_count - 1;
			current_top_item = template_count - 1;
		}
		else
			current_top_item -= 1;
	}

	//going down
	if(newselection > (current_top_item + 2) || newselection >= template_count)
	{
		if(newselection >= template_count)
		{
			newselection = 0;
			current_top_item = 0;
		}
		else
			current_top_item += 1;
	}

	selected_item = newselection;
}

static void template_list_select(struct nokia_ui *ui)
{
	*template_list_arguments.selected_template_id = templates[selected_item].id;
	*template_list_arguments.selected_template_index = selected_item;
	screen_set_current(ui, template_list_arguments.button_return_screen, template_list_arguments.button_return_screen_arguments);
}
