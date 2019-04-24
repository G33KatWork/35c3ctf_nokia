#include "screens/sms_inbox_list.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "db.h"
#include "display.h"
#include "keyboard.h"
#include "fonts.h"

#include "generated/font_small_bold.h"
#include "generated/nokia_images.h"

#include "screens/sms_inbox.h"
#include "screens/sms_viewer.h"
#include "screens/menu.h"
#include "screens/info.h"

#include "menus/main_menu.h"

struct sms_message {
	int id;
	int read;
	char* text;
	char* sender;
	char* sender_name;
	int date;
};

struct screen screen_sms_inbox_delete;

static struct screen_sms_inbox_list_arguments sms_inbox_list_arguments;

static int message_count;
static struct sms_message* messages;
static int current_top_item;
static int selected_item;

static void sms_inbox_draw_scrollbar(struct nokia_ui *ui);
static void sms_inbox_select_entry(int newselection);
static void sms_inbox_open_selected_message(struct nokia_ui *ui);

static void sms_inbox_list_enter(struct nokia_ui *ui, void* arguments)
{
	assert(arguments);
	memcpy(&sms_inbox_list_arguments, arguments, sizeof(sms_inbox_list_arguments));

	current_top_item = 0;
	selected_item = 0;

	message_count = db_get_message_entry_count();

	messages = malloc(message_count * sizeof(struct sms_message));
	for(int i = 0; i < message_count; i++)
	{
		messages[i].text = calloc(1, 161);
		messages[i].sender = calloc(1, 21);
		messages[i].sender_name = calloc(1, 21);

		db_get_message(i, &messages[i].id, messages[i].sender, 20, messages[i].text, 161, &messages[i].read, &messages[i].date);
		db_get_phone_book_name_by_number(messages[i].sender, messages[i].sender_name, 20);
	}
}

static void sms_inbox_list_leave()
{
	for(int i = 0; i < message_count; i++)
	{
		free(messages[i].text);
		free(messages[i].sender);
		free(messages[i].sender_name);
	}

	free(messages);
}

static void sms_inbox_list_draw(struct nokia_ui *ui)
{
	int xpos = 8, ypos = 0;
	char buf[20] = {0};

	snprintf(buf, sizeof(buf), "%d", selected_item+1);
	xpos = DISPLAY_WIDTH - fonts_get_pixel_width(font_3310_small_bold, buf);
	fonts_draw_text(ui, font_3310_small_bold, buf, xpos, 0);

	xpos = 0;
	ypos = 9;

	for(int i = current_top_item; i < current_top_item+3 && i < message_count; i++)
	{
		if(messages[i].read)
			display_blit_image(ui, 0, ypos, &sms_read);
		else
			display_blit_image(ui, 0, ypos, &sms_unread);

		fonts_draw_text(ui, font_3310_small_bold, messages[i].sender_name[0] != '\0' ? messages[i].sender_name : messages[i].sender, xpos+1+8, ypos);
		if(i == selected_item)
			display_invert_box(ui, 8, ypos-1, DISPLAY_WIDTH-5-8, fonts_get_pixel_height(font_3310_small_bold)+1);

		ypos += fonts_get_pixel_height(font_3310_small_bold) + 2;
	}

	xpos = DISPLAY_WIDTH/2 - fonts_get_pixel_width(font_3310_small_bold, sms_inbox_list_arguments.button_text)/2;
	ypos = DISPLAY_HEIGHT - fonts_get_pixel_height(font_3310_small_bold);
	fonts_draw_text(ui, font_3310_small_bold, sms_inbox_list_arguments.button_text, xpos, ypos);

	sms_inbox_draw_scrollbar(ui);
}

static void sms_inbox_list_handle_input(struct nokia_ui *ui, int keys)
{
	switch(keys)
	{
		case NOKIA_KEY_ESC:
			screen_set_current(ui, sms_inbox_list_arguments.return_screen, sms_inbox_list_arguments.return_screen_arguments);
			break;

		case NOKIA_KEY_ENTER:
			sms_inbox_open_selected_message(ui);
			break;

		case NOKIA_KEY_DOWN:
			sms_inbox_select_entry(selected_item+1);
			break;

		case NOKIA_KEY_UP:
			sms_inbox_select_entry(selected_item-1);
			break;
	}
}

struct screen screen_sms_inbox_list = {
	.enter_callback = sms_inbox_list_enter,
	.leave_callback = sms_inbox_list_leave,
	.draw_callback = sms_inbox_list_draw,
	.handle_input = sms_inbox_list_handle_input,
	.handle_baseband = screen_handle_baseband_null,
};


static void sms_inbox_draw_scrollbar(struct nokia_ui *ui)
{
	int xpos = 0, ypos = 0;

	if(message_count > 0)
	{
		ypos = 11;

		if(message_count > 1)
			ypos += (22 / (message_count-1)) * selected_item;

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


static void sms_inbox_select_entry(int newselection)
{
	//going up
	if(newselection < current_top_item)
	{
		if(newselection < 0)
		{
			newselection = message_count - 1;
			current_top_item = message_count - 1;
		}
		else
			current_top_item -= 1;
	}

	//going down
	if(newselection > (current_top_item + 2) || newselection >= message_count)
	{
		if(newselection >= message_count)
		{
			newselection = 0;
			current_top_item = 0;
		}
		else
			current_top_item += 1;
	}

	selected_item = newselection;
}

static char sms_view_text[460];
static char sms_view_number[DB_PHONEBOOK_NUMBER_MAX_LEN+1];
static char sms_view_sender_name[DB_PHONEBOOK_NAME_MAX_LEN+1];
static int sms_view_date;
static int sms_view_id;

static void sms_inbox_open_selected_message(struct nokia_ui *ui)
{
	db_get_message(selected_item, &sms_view_id, sms_view_number, sizeof(sms_view_number), sms_view_text, sizeof(sms_view_text)-1, NULL, &sms_view_date);
	db_get_phone_book_name_by_number(sms_view_number, sms_view_sender_name, sizeof(sms_view_sender_name));
	deb_set_message_read(sms_view_id);

	screen_set_current(ui, &screen_sms_viewer, &(struct sms_viewer_arguments) {
		.text = sms_view_text,
		.number = sms_view_number,
		.sender_name = sms_view_sender_name,
		.date = &sms_view_date,

		.button_text = "Delete",

		.return_screen = &screen_sms_inbox,
		.return_screen_arguments = NULL,

		.button_screen = &screen_sms_inbox_delete,
		.button_screen_arguments = &sms_view_id
	});
}





static void sms_inbox_delete_enter(struct nokia_ui *ui, void* arguments)
{
	assert(arguments);
	int* sms_id = arguments;

	db_del_message(*sms_id);

	screen_set_current(ui, &screen_info, &(struct screen_info_arguments){
		.text_line1 = "Memory",
		.text_line2 = "erased",
		.text_line3 = "",
		.animation = &erase_anim,
		.return_screen = &screen_sms_inbox,
		.return_screen_arguments = NULL
	});
}


struct screen screen_sms_inbox_delete = {
	.enter_callback = sms_inbox_delete_enter,
	.leave_callback = screen_leave_null,
	.draw_callback = screen_draw_null,
	.handle_input = screen_handle_input_null,
	.handle_baseband = screen_handle_baseband_null,
};
