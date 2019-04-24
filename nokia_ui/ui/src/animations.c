#include "animations.h"

#include <stddef.h>

#include "display.h"
#include "ui.h"

#include "generated/nokia_images.h"

#define MAX_ANIMATIONS 2

struct animation startup_anim = {
	.x = 0,
	.y = 0,
	.framecount = 10,
	.images = {&startup1, &startup2, &startup3, &startup4, &startup5, &startup6, &startup7, &startup7, &startup7, &startup7},
	//.frame_delay = 3,
	.frame_delay = 1,
	.cur_animation_step = 0
};

struct animation menu_phnone_book_anim = {
	.x = 10,
	.y = 22,
	.framecount = 13,
	.images = {&menu_phone_book1, &menu_phone_book1, &menu_phone_book1, &menu_phone_book1, &menu_phone_book1, &menu_phone_book2, &menu_phone_book3, &menu_phone_book4, &menu_phone_book1, &menu_phone_book2, &menu_phone_book3, &menu_phone_book4, &menu_phone_book1},
	.frame_delay = 4,
	.cur_animation_step = 0
};

struct animation menu_sms_anim = {
	.x = 10,
	.y = 22,
	.framecount = 21,
	.images = {&menu_sms1, &menu_sms1, &menu_sms1, &menu_sms1, &menu_sms1, &menu_sms2, &menu_sms3, &menu_sms4, &menu_sms5, &menu_sms6, &menu_sms7, &menu_sms8, &menu_sms9, &menu_sms10, &menu_sms11, &menu_sms12, &menu_sms13, &menu_sms14, &menu_sms13, &menu_sms12, &menu_sms1},
	.frame_delay = 4,
	.cur_animation_step = 0
};

struct animation menu_settings_anim = {
	.x = 10,
	.y = 22,
	.framecount = 12,
	.images = {&menu_settings1, &menu_settings1, &menu_settings1, &menu_settings1, &menu_settings1, &menu_settings2, &menu_settings3, &menu_settings4, &menu_settings5, &menu_settings6, &menu_settings7, &menu_settings8},
	.frame_delay = 4,
	.cur_animation_step = 0
};

struct animation success_anim = {
	.x = DISPLAY_WIDTH-22,
	.y = 0,
	.framecount = 5,
	.images = {&success1, &success2, &success3, &success3, &success3},
	.frame_delay = 6,
	.cur_animation_step = 0
};

struct animation info_anim = {
	.x = DISPLAY_WIDTH-22,
	.y = 0,
	.framecount = 5,
	.images = {&info1, &info1, &info1, &info1, &info1},
	.frame_delay = 6,
	.cur_animation_step = 0
};

struct animation error_anim = {
	.x = DISPLAY_WIDTH-22,
	.y = 0,
	.framecount = 14,
	.images = {&error1, &error1, &error1, &error1, &error1, &error1, &error1, &error1, &error1, &error1, &error1, &error1, &error1, &error1},
	.frame_delay = 6,
	.cur_animation_step = 0
};

struct animation sms_anim = {
	.x = DISPLAY_WIDTH-22,
	.y = 0,
	.framecount = 14,
	.images = {&sms_send1, &sms_send2, &sms_send3, &sms_send4, &sms_send5, &sms_send6, &sms_send7, &sms_send8, &sms_send9, &sms_send10, &sms_send11, &sms_send12, &sms_send13, &sms_send14},
	.frame_delay = 6,
	.cur_animation_step = 0
};

struct animation erase_anim = {
	.x = DISPLAY_WIDTH-22,
	.y = 0,
	.framecount = 11,
	.images = {&erase1, &erase2, &erase3, &erase4, &erase5, &erase6, &erase2, &erase3, &erase4, &erase2, &erase1},
	.frame_delay = 6,
	.cur_animation_step = 0
};

struct animation smile_anim = {
	.x = DISPLAY_WIDTH-22,
	.y = 0,
	.framecount = 4,
	.images = {&smile1, &smile2, &smile3, &smile4},
	.frame_delay = 6,
	.cur_animation_step = 0
};

static struct animation* running_animations[MAX_ANIMATIONS] = {0};

void animation_queue(int slot, struct animation* animation)
{
	if(slot >= MAX_ANIMATIONS || !animation)
		return;

	running_animations[slot] = animation;
	animation->cur_animation_step = 0;
}

void animation_clear()
{
	for(int i = 0; i < MAX_ANIMATIONS; i++)
		running_animations[i] = NULL;
}

_Bool animation_isdone(int slot)
{
	if(slot >= MAX_ANIMATIONS)
		return true;

	return !running_animations[slot] || ((running_animations[slot]->framecount * running_animations[slot]->frame_delay)-1 == running_animations[slot]->cur_animation_step);
}

void animation_play(struct nokia_ui* ui)
{
	for(int i = 0; i < MAX_ANIMATIONS; i++)
	{
		if(running_animations[i])
		{
			struct animation* cur_anim = running_animations[i];
			uint8_t frameToPlay = cur_anim->cur_animation_step / cur_anim->frame_delay;

			display_blit_image(ui, cur_anim->x, cur_anim->y, cur_anim->images[frameToPlay]);

			if(!animation_isdone(i))
				running_animations[i]->cur_animation_step++;
		}
	}
}
