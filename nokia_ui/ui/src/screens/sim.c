#include "screens/sim.h"

#include <string.h>
#include <assert.h>

#include "animations.h"
#include "uictl.h"
#include "ui_event.h"
#include "screens/error.h"
#include "screens/pin_enter.h"
#include "screens/info.h"
#include "screens/main_screen.h"

struct screen screen_sim_check_pin;
struct screen screen_sim_unblock_pin;

int pin_len = 0;
char pin[8];

int puk_len = 0;
char puk[8];

// ################################################################################################################################

static void sim_enter_pin_screen_enter(struct nokia_ui *ui, void* arguments)
{
    pin_len = 0;
    memset(pin, 0, sizeof(pin));

    screen_set_current(ui, &screen_pin_enter, &(struct screen_pin_enter_arguments){
        .max_input_len = 8,
        .min_input_len = 4,
        .input_buffer = pin,
        .line1_text = "Enter",
        .line2_text = "PIN code:",
        .button_text = "OK",
        .entered_text_len = &pin_len,

        .success_return_screen = &screen_sim_check_pin,
        .success_return_screen_arguments = NULL
    });
}

struct screen screen_sim_enter_pin = {
    .enter_callback = sim_enter_pin_screen_enter,
    .leave_callback = screen_leave_null,
    .draw_callback = screen_draw_null,
    .handle_input = screen_handle_input_null,
    .handle_baseband = screen_handle_baseband_null,
};

// ################################################################################################################################

static struct screen_pin_enter_arguments enter_new_pin_args = {
    .max_input_len = 8,
        .min_input_len = 4,
    .input_buffer = pin,
    .line1_text = "Enter new",
    .line2_text = "PIN code:",
    .button_text = "OK",
    .entered_text_len = &pin_len,

    .success_return_screen = &screen_sim_unblock_pin,
    .success_return_screen_arguments = NULL
};

static void sim_enter_puk_screen_enter(struct nokia_ui *ui, void* arguments)
{
    puk_len = 0;
    memset(puk, 0, sizeof(puk));

    pin_len = 0;
    memset(pin, 0, sizeof(pin));

    screen_set_current(ui, &screen_pin_enter, &(struct screen_pin_enter_arguments){
        .max_input_len = 8,
        .min_input_len = 8,
        .input_buffer = puk,
        .line1_text = "Enter",
        .line2_text = "PUK code:",
        .button_text = "OK",
        .entered_text_len = &puk_len,

        .success_return_screen = &screen_pin_enter,
        .success_return_screen_arguments = &enter_new_pin_args
    });
}

struct screen screen_sim_enter_puk = {
    .enter_callback = sim_enter_puk_screen_enter,
    .leave_callback = screen_leave_null,
    .draw_callback = screen_draw_null,
    .handle_input = screen_handle_input_null,
    .handle_baseband = screen_handle_baseband_null,
};

// ################################################################################################################################

static void sim_check_pin_enter(struct nokia_ui *ui, void* arguments)
{
    uictl_send_pin(ui, pin, NULL, PRIM_SIM_ENTER_PIN);
}

static void sim_check_pin_handle_baseband(struct nokia_ui *ui, int event, void* data1, void* data2)
{
    struct ui_event_sim_params *sim_params = data1;

    if(sim_params->cause == PRIM_SIM_VALID_UNLOCKED)
        screen_set_current(ui, &screen_info, &(struct screen_info_arguments){
            .text_line1 = "Code",
            .text_line2 = "accepted",
            .text_line3 = "",
            .animation = &smile_anim,
            .return_screen = &screen_main_screen,
            .return_screen_arguments = NULL
        });
    else if(sim_params->cause == PRIM_SIM_PIN1_REQUIRED)
        screen_set_current(ui, &screen_info, &(struct screen_info_arguments){
            .text_line1 = "Code",
            .text_line2 = "error",
            .text_line3 = "",
            .animation = &error_anim,
            .return_screen = &screen_sim_enter_pin,
            .return_screen_arguments = NULL
        });
    else if(sim_params->cause == PRIM_SIM_PIN1_BLOCKED)
        screen_set_current(ui, &screen_info, &(struct screen_info_arguments){
            .text_line1 = "SIM card",
            .text_line2 = "blocked",
            .text_line3 = "",
            .animation = &error_anim,
            .return_screen = &screen_sim_enter_puk,
            .return_screen_arguments = NULL
        });
    else if(sim_params->cause == PRIM_SIM_PUC_BLOCKED)
        screen_set_current(ui, &screen_error, &(struct screen_error_arguments){.text_line1="SIM card", .text_line2="is blocked", .text_line3=""});
    else if(sim_params->cause == PRIM_SIM_INVALID_MISSING)
        screen_set_current(ui, &screen_error, &(struct screen_error_arguments){.text_line1="SIM invalid", .text_line2="or missing", .text_line3=""});
}

struct screen screen_sim_check_pin = {
    .enter_callback = sim_check_pin_enter,
    .leave_callback = screen_leave_null,
    .draw_callback = screen_draw_null,
    .handle_input = screen_handle_input_null,
    .handle_baseband = sim_check_pin_handle_baseband,
};

// ################################################################################################################################

static void sim_check_unblock_enter(struct nokia_ui *ui, void* arguments)
{
    uictl_send_pin(ui, puk, pin, PRIM_SIM_ENTER_PUC);
}

static void sim_unblock_pin_handle_baseband(struct nokia_ui *ui, int event, void* data1, void* data2)
{
    struct ui_event_sim_params *sim_params = data1;

    if(sim_params->cause == PRIM_SIM_VALID_UNLOCKED)
        screen_set_current(ui, &screen_info, &(struct screen_info_arguments){
            .text_line1 = "Code",
            .text_line2 = "accepted",
            .text_line3 = "",
            .animation = &smile_anim,
            .return_screen = &screen_main_screen,
            .return_screen_arguments = NULL
        });
    else if(sim_params->cause == PRIM_SIM_PIN1_BLOCKED)
        screen_set_current(ui, &screen_info, &(struct screen_info_arguments){
            .text_line1 = "Code",
            .text_line2 = "error",
            .text_line3 = "",
            .animation = &error_anim,
            .return_screen = &screen_sim_enter_puk,
            .return_screen_arguments = NULL
        });
    else if(sim_params->cause == PRIM_SIM_PUC_BLOCKED)
        screen_set_current(ui, &screen_error, &(struct screen_error_arguments){.text_line1="SIM card", .text_line2="is blocked", .text_line3=""});
    else if(sim_params->cause == PRIM_SIM_INVALID_MISSING)
        screen_set_current(ui, &screen_error, &(struct screen_error_arguments){.text_line1="SIM invalid", .text_line2="or missing", .text_line3=""});
}

struct screen screen_sim_unblock_pin = {
    .enter_callback = sim_check_unblock_enter,
    .leave_callback = screen_leave_null,
    .draw_callback = screen_draw_null,
    .handle_input = screen_handle_input_null,
    .handle_baseband = sim_unblock_pin_handle_baseband,
};
