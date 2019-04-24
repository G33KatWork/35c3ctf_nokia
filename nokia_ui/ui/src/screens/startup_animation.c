#include "screens/startup_animation.h"

#include "screens/error.h"
#include "screens/sim.h"
#include "screens/main_screen.h"
#include "animations.h"
#include "ui_event.h"
#include "primitives.h"

#include <stdlib.h>

int sim_received_cause = -1;

static void startup_animation_enter(struct nokia_ui *ui, void* arguments)
{
	animation_queue(0, &startup_anim);
}

static void startup_animation_leave()
{
	animation_clear();
}

static void startup_animation_draw(struct nokia_ui *ui)
{
    if(animation_isdone(0) && sim_received_cause != -1)
    {
        if(sim_received_cause == PRIM_SIM_VALID_UNLOCKED)
            screen_set_current(ui, &screen_main_screen, NULL);
        else if(sim_received_cause == PRIM_SIM_INVALID_MISSING)
            screen_set_current(ui, &screen_error, &(struct screen_error_arguments){.text_line1="SIM invalid", .text_line2="or missing", .text_line3=""});
        else if(sim_received_cause == PRIM_SIM_PIN1_REQUIRED)
            screen_set_current(ui, &screen_sim_enter_pin, NULL);
        else if(sim_received_cause == PRIM_SIM_PIN1_BLOCKED)
            screen_set_current(ui, &screen_sim_enter_puk, NULL);
    }
}

static void startup_animation_handle_baseband(struct nokia_ui *ui, int event, void* data1, void* data2)
{
    struct ui_event_sim_params *sim_params = data1;
    sim_received_cause = sim_params->cause;
}

struct screen screen_startup_animation = {
	.enter_callback = startup_animation_enter,
	.leave_callback = startup_animation_leave,
	.draw_callback = startup_animation_draw,
	.handle_input = screen_handle_input_null,
    .handle_baseband = startup_animation_handle_baseband,
};
