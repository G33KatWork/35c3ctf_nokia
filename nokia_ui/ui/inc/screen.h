#ifndef _SCREEN_H_
#define _SCREEN_H_

#include "nokia_ui.h"

struct screen {
	void (*enter_callback)(struct nokia_ui *ui, void* arguments);
	void (*leave_callback)(struct nokia_ui *ui);
	void (*draw_callback)(struct nokia_ui *ui);
	void (*handle_input)(struct nokia_ui *ui, int keys);
    void (*handle_baseband)(struct nokia_ui *ui, int event, void* data1, void* data2);
};

void screen_set_current(struct nokia_ui *ui, struct screen* new_screen, void* arguments);

void screen_handle_input(struct nokia_ui *ui, int keys);
void screen_handle_baseband(struct nokia_ui *ui, int event, void* data1, void* data2);
void screen_draw(struct nokia_ui *ui);


void screen_enter_null(struct nokia_ui *ui, void* arguments);
void screen_leave_null(struct nokia_ui *ui);
void screen_draw_null(struct nokia_ui *ui);
void screen_handle_input_null(struct nokia_ui *ui, int keys);
void screen_handle_baseband_null(struct nokia_ui *ui, int event, void* data1, void* data2);

#endif
