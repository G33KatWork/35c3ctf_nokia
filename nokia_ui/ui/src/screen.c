#include "screen.h"

#include <assert.h>

void screen_set_current(struct nokia_ui *ui, struct screen* new_screen, void* arguments)
{
	assert(new_screen);

	if(ui->ui_entity.screen_current)
		ui->ui_entity.screen_current->leave_callback(ui);

	ui->ui_entity.screen_current = new_screen;
	ui->ui_entity.screen_current->enter_callback(ui, arguments);
}

void screen_handle_input(struct nokia_ui *ui, int keys)
{
	assert(ui->ui_entity.screen_current);
	ui->ui_entity.screen_current->handle_input(ui, keys);
}

void screen_handle_baseband(struct nokia_ui *ui, int event, void* data1, void* data2)
{
    assert(ui->ui_entity.screen_current);
    ui->ui_entity.screen_current->handle_baseband(ui, event, data1, data2);
}

void screen_draw(struct nokia_ui *ui)
{
	assert(ui->ui_entity.screen_current);
	ui->ui_entity.screen_current->draw_callback(ui);
}


void screen_enter_null(struct nokia_ui *ui, void* arguments){}
void screen_leave_null(struct nokia_ui *ui){}
void screen_draw_null(struct nokia_ui *ui){}
void screen_handle_input_null(struct nokia_ui *ui, int keys){}
void screen_handle_baseband_null(struct nokia_ui *ui, int event, void* data1, void* data2){}
