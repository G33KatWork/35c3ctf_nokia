#ifndef _UI_H_
#define _UI_H_

#include "nokia_ui.h"

#define UI_FPS 30

int ui_init(struct nokia_ui* ui);
void ui_exit(struct nokia_ui* ui);
void ui_render(struct nokia_ui* ui);

void ui_display_clear(struct nokia_ui* ui);
void ui_display_set_pixel(struct nokia_ui* ui, int x, int y);
void ui_display_clear_pixel(struct nokia_ui* ui, int x, int y);
void ui_display_invert_pixel(struct nokia_ui* ui, int x, int y);

#endif
