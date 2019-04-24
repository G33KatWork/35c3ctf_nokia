#include "screens/error.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <alloca.h>

#include "display.h"
#include "fonts.h"

#include "generated/font_small_bold.h"

static struct screen_error_arguments error_arguments;

static void error_enter(struct nokia_ui *ui, void* arguments)
{
    assert(arguments);
    memcpy(&error_arguments, arguments, sizeof(struct screen_error_arguments));
}

static void error_draw(struct nokia_ui *ui)
{
    //draw enter pin text
    fonts_draw_text(ui, font_3310_small_bold, error_arguments.text_line1, 0, 8);
    fonts_draw_text(ui, font_3310_small_bold, error_arguments.text_line2, 0, 8 + fonts_get_pixel_height(font_3310_small_bold));
    fonts_draw_text(ui, font_3310_small_bold, error_arguments.text_line3, 0, 8 + 2*fonts_get_pixel_height(font_3310_small_bold));
}

struct screen screen_error = {
    .enter_callback = error_enter,
    .leave_callback = screen_leave_null,
    .draw_callback = error_draw,
    .handle_input = screen_handle_input_null,
    .handle_baseband = screen_handle_baseband_null,
};
