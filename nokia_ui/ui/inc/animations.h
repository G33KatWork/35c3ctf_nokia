#ifndef _ANIMATIONS_H_
#define _ANIMATIONS_H_

#include <stdint.h>

#include "nokia_ui.h"
#include "images.h"

//FIXME: keep animation state in nokia_ui struct

struct animation {
	uint8_t x;
	uint8_t y;
	uint8_t framecount;
	uint32_t frame_delay;
	uint32_t cur_animation_step;
	const struct image* images[];
};

extern struct animation startup_anim;
extern struct animation menu_phnone_book_anim;
extern struct animation menu_sms_anim;
extern struct animation menu_settings_anim;

extern struct animation success_anim;
extern struct animation info_anim;
extern struct animation error_anim;
extern struct animation sms_anim;
extern struct animation erase_anim;
extern struct animation smile_anim;

void animation_queue(int slot, struct animation* animation);
void animation_clear(void);
_Bool animation_isdone(int slot);
void animation_play(struct nokia_ui* ui);

#endif
