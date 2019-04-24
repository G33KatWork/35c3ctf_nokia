#include "screens/main_screen.h"

#include <sys/time.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#include "db.h"
#include "display.h"
#include "keyboard.h"
#include "fonts.h"
#include "wordwrap.h"
#include "networks.h"
#include "ui_event.h"
#include "menus/main_menu.h"
#include "screens/menu.h"
#include "screens/sms_inbox.h"
#include "screens/progress.h"

#include "generated/font_small_bold.h"
#include "generated/font_small_plain.h"
#include "generated/nokia_images.h"

const char* button_text;

static void main_screen_draw(struct nokia_ui *ui)
{
	display_blit_image(ui, 0, 31, &reception);

	if(ui->ui_entity.on_network)
	{
		display_blit_image(ui, 0, 0, &bar4);
		display_blit_image(ui, 0, 8, &bar3);
		display_blit_image(ui, 0, 16, &bar2);
		display_blit_image(ui, 0, 24, &bar1);
	}

	display_blit_image(ui, 80, 31, &battery);
	display_blit_image(ui, 80, 0, &bar4);
	display_blit_image(ui, 81, 8, &bar3);
	display_blit_image(ui, 82, 16, &bar2);
	display_blit_image(ui, 82, 24, &bar1);

	if(ui->ui_entity.unread_sms > 0)
	{
		button_text = "Read";

		fonts_draw_int(ui, font_3310_small_bold, ui->ui_entity.unread_sms, 7, 10);
		fonts_draw_text(ui, font_3310_small_bold, ui->ui_entity.unread_sms == 1 ? "message" : "messages", 7, 10 + fonts_get_pixel_height(font_3310_small_bold));
		fonts_draw_text(ui, font_3310_small_bold, "received", 7, 10 + 2*fonts_get_pixel_height(font_3310_small_bold));
	}
	else
	{
		button_text = "Menu";

		if(ui->ui_entity.on_network)
		{
			const char* network_name = gsm_get_mnc(ui->ui_entity.mcc, ui->ui_entity.mnc);
			int providerx = DISPLAY_WIDTH/2 - fonts_get_pixel_width(font_3310_small_bold, network_name) / 2;
			int providery = 14;
			fonts_draw_text(ui, font_3310_small_bold, network_name, providerx, providery);
		}
	}

	if(db_get_unread_message_count() > 0)
		display_blit_image(ui, 7, 0, &unread_sms);

	int buttonx = DISPLAY_WIDTH/2 - fonts_get_pixel_width(font_3310_small_bold, button_text) / 2;
	int buttony = DISPLAY_HEIGHT - fonts_get_pixel_height(font_3310_small_bold);
	fonts_draw_text(ui, font_3310_small_bold, button_text, buttonx, buttony);

	struct timeval tv;
	time_t curtime;
	gettimeofday(&tv, NULL);
	curtime=tv.tv_sec;

	char timestr[10];
	strftime(timestr, sizeof(timestr), "%H:%M", localtime(&curtime));

	int clockx = 50;
	int clocky = 0;
	fonts_draw_text(ui, font_3310_small_bold, timestr, clockx, clocky);
}

static void main_screen_handle_input(struct nokia_ui *ui, int keys)
{
	switch(keys)
	{
		case NOKIA_KEY_ENTER:
			if(ui->ui_entity.unread_sms > 0)
			{
				ui->ui_entity.unread_sms = 0;
				screen_set_current(ui, &screen_sms_inbox, NULL);
			}
			else
				screen_set_current(ui, &screen_menu, &main_menu_select_first_item);
			break;

		case '0' ... '9':
			break;

		case NOKIA_KEY_BACKSPACE:
			if(ui->ui_entity.unread_sms > 0)
				ui->ui_entity.unread_sms = 0;
			break;
	}
}

struct screen screen_main_screen = {
	.enter_callback = screen_enter_null,
	.leave_callback = screen_leave_null,
	.draw_callback = main_screen_draw,
	.handle_input = main_screen_handle_input,
	.handle_baseband = screen_handle_baseband_null,
};
