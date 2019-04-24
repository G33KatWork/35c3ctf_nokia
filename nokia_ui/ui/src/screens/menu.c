#include "screens/menu.h"

#include <assert.h>
#include <string.h>
#include <string.h>

#include "animations.h"
#include "display.h"
#include "keyboard.h"
#include "fonts.h"

#include "generated/nokia_images.h"
#include "generated/font_large_bold.h"
#include "generated/font_small_bold.h"

#include "screens/main_screen.h"

static struct screen_menu_arguments menu_arguments;

static void menu_enter_selected(struct nokia_ui *ui);
static void menu_select_entry(int newselection);
static _Bool menu_goto_parent(void);
static void menu_draw_scrollbar(struct nokia_ui *ui);

static void menu_screen_enter(struct nokia_ui *ui, void* arguments)
{
	assert(arguments);
	memcpy(&menu_arguments, arguments, sizeof(struct screen_menu_arguments));

	if(menu_arguments.submenu_to_set)
		menu_arguments.menu_instance->menu = menu_arguments.submenu_to_set;

	if(menu_arguments.item_to_select >= 0)
		menu_select_entry(menu_arguments.item_to_select);
	else
		menu_select_entry(menu_arguments.menu_instance->menu->selected_entry);
}

static void menu_screen_leave()
{
	animation_clear();
}

static void menu_screen_draw(struct nokia_ui *ui)
{
	assert(menu_arguments.menu_instance);
	assert(menu_arguments.menu_instance->menu);

	int xpos = 0, ypos = 0;

	char buf[20] = {0};

	switch(menu_arguments.menu_instance->menu->type)
	{
		case MENU_TYPE_MAIN:
			snprintf(buf, sizeof(buf), "%d", menu_arguments.menu_instance->menu->selected_entry+1);
			xpos = DISPLAY_WIDTH - fonts_get_pixel_width(font_3310_small_bold, buf);
			fonts_draw_text(ui, font_3310_small_bold, buf, xpos, 0);

			xpos = DISPLAY_WIDTH/2 - fonts_get_pixel_width(font_3310_small_bold, "Select")/2;
			ypos = DISPLAY_HEIGHT - fonts_get_pixel_height(font_3310_small_bold);
			fonts_draw_text(ui, font_3310_small_bold, "Select", xpos, ypos);

			xpos = DISPLAY_WIDTH/2 - fonts_get_pixel_width(font_3310_small_bold, menu_arguments.menu_instance->menu->entries[menu_arguments.menu_instance->menu->selected_entry].name)/2 - 6;
			ypos = 5;
			fonts_draw_text(ui, font_3310_large_bold, menu_arguments.menu_instance->menu->entries[menu_arguments.menu_instance->menu->selected_entry].name, xpos, ypos);

			menu_draw_scrollbar(ui);

			break;

		case MENU_TYPE_SUB:
			snprintf(buf, sizeof(buf), "%d", menu_arguments.menu_instance->menu->selected_entry+1);
			xpos = DISPLAY_WIDTH - fonts_get_pixel_width(font_3310_small_bold, buf);
			fonts_draw_text(ui, font_3310_small_bold, buf, xpos, 0);

			xpos = 0;
			ypos = 9;

			for(int i = menu_arguments.menu_instance->menu->submenu_current_top_item; i < menu_arguments.menu_instance->menu->submenu_current_top_item+3 && i < menu_arguments.menu_instance->menu->num_entries; i++)
			{
				fonts_draw_text(ui, font_3310_small_bold, menu_arguments.menu_instance->menu->entries[i].name, xpos+1, ypos);
				if(i == menu_arguments.menu_instance->menu->selected_entry)
					display_invert_box(ui, 0, ypos-1, DISPLAY_WIDTH-5, fonts_get_pixel_height(font_3310_small_bold)+1);

				ypos += fonts_get_pixel_height(font_3310_small_bold) + 2;
			}

			xpos = DISPLAY_WIDTH/2 - fonts_get_pixel_width(font_3310_small_bold, "Select")/2;
			ypos = DISPLAY_HEIGHT - fonts_get_pixel_height(font_3310_small_bold);
			fonts_draw_text(ui, font_3310_small_bold, "Select", xpos, ypos);

			menu_draw_scrollbar(ui);

			break;

		case MENU_TYPE_SPECIAL:
		break;
	}
}

static void menu_screen_handle_input(struct nokia_ui *ui, int keys)
{
	assert(menu_arguments.menu_instance);

	switch(keys)
	{
		case NOKIA_KEY_ESC:
			//if menu_goto_parent() returns false, we have no parent menu, so go to return screen
			if(!menu_goto_parent())
				screen_set_current(ui, menu_arguments.menu_instance->return_screen, menu_arguments.menu_instance->return_screen_arguments);
			break;

		case NOKIA_KEY_ENTER:
			menu_enter_selected(ui);
			break;

		case NOKIA_KEY_DOWN:
			menu_select_entry(menu_arguments.menu_instance->menu->selected_entry+1);
			break;

		case NOKIA_KEY_UP:
			menu_select_entry(menu_arguments.menu_instance->menu->selected_entry-1);
			break;
	}
}

static void menu_enter_selected(struct nokia_ui *ui)
{
	assert(menu_arguments.menu_instance);
	assert(menu_arguments.menu_instance->menu);

	struct menu* menu_to_enter = menu_arguments.menu_instance->menu->entries[menu_arguments.menu_instance->menu->selected_entry].sub_menu;

	//enter a special screen
	if(menu_to_enter->type == MENU_TYPE_SPECIAL)
	{
		screen_set_current(ui, menu_to_enter->special_screen, menu_to_enter->arguments);
		return;
	}

	//enter a submenu
	assert(menu_arguments.menu_instance->menu->selected_entry < menu_arguments.menu_instance->menu->num_entries);
	assert(menu_to_enter != NULL);

	menu_arguments.menu_instance->menu->submenu_current_top_item = 0;
	menu_arguments.menu_instance->menu = menu_to_enter;
	animation_clear();
	menu_select_entry(0);
}

static void menu_select_entry(int newselection)
{
	assert(menu_arguments.menu_instance != NULL);
	assert(menu_arguments.menu_instance->menu != NULL);

	if(menu_arguments.menu_instance->menu->type == MENU_TYPE_MAIN)
	{
		if(newselection < 0)
			newselection = menu_arguments.menu_instance->menu->num_entries - 1;

		if(newselection >= menu_arguments.menu_instance->menu->num_entries)
			newselection = 0;

		menu_arguments.menu_instance->menu->selected_entry = newselection;

		if(menu_arguments.menu_instance->menu->entries[newselection].animation)
			animation_queue(0, menu_arguments.menu_instance->menu->entries[newselection].animation);
	}
	else
	{
		//going up
		if(newselection < menu_arguments.menu_instance->menu->submenu_current_top_item)
		{
			if(newselection < 0)
			{
				newselection = menu_arguments.menu_instance->menu->num_entries - 1;
				menu_arguments.menu_instance->menu->submenu_current_top_item = menu_arguments.menu_instance->menu->num_entries - 1;
			}
			else
				menu_arguments.menu_instance->menu->submenu_current_top_item -= 1;
		}

		//going down
		if(newselection > (menu_arguments.menu_instance->menu->submenu_current_top_item + 2) || newselection >= menu_arguments.menu_instance->menu->num_entries)
		{
			if(newselection >= menu_arguments.menu_instance->menu->num_entries)
			{
				newselection = 0;
				menu_arguments.menu_instance->menu->submenu_current_top_item = 0;
			}
			else
				menu_arguments.menu_instance->menu->submenu_current_top_item += 1;
		}

		menu_arguments.menu_instance->menu->selected_entry = newselection;
	}
}

static _Bool menu_goto_parent()
{
	assert(menu_arguments.menu_instance);
	assert(menu_arguments.menu_instance->menu);

	menu_arguments.menu_instance->menu->submenu_current_top_item = menu_arguments.menu_instance->menu->selected_entry = 0;
	animation_clear();

	//if(menu_arguments.menu_instance->menu->type == MENU_TYPE_SPECIAL)
	//	screen_set_current(&screen_menu, NULL);

	if(menu_arguments.menu_instance->menu->parent_menu)
	{
		//set new menu
		menu_arguments.menu_instance->menu = menu_arguments.menu_instance->menu->parent_menu;

		//reselecting current item requeues the animation
		menu_select_entry(menu_arguments.menu_instance->menu->selected_entry);

		return true;
	}
	else
		return false;
}

static void menu_draw_scrollbar(struct nokia_ui *ui)
{
	int xpos = 0, ypos = 0;

	if(menu_arguments.menu_instance->menu->num_entries > 0)
	{
		ypos = 11;

		if(menu_arguments.menu_instance->menu->num_entries > 1)
			ypos += (22 / (menu_arguments.menu_instance->menu->num_entries-1)) * menu_arguments.menu_instance->menu->selected_entry;

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


struct screen screen_menu = {
	.enter_callback = menu_screen_enter,
	.leave_callback = menu_screen_leave,
	.draw_callback = menu_screen_draw,
	.handle_input = menu_screen_handle_input,
	.handle_baseband = screen_handle_baseband_null,
};
