#include "screens/pin_enter.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <alloca.h>

#include "display.h"
#include "keyboard.h"
#include "fonts.h"

#include "generated/nokia_images.h"
#include "generated/font_small_bold.h"

static struct screen_pin_enter_arguments pin_enter_arguments;

static void pin_enter_enter(struct nokia_ui *ui, void* arguments)
{
    assert(arguments);
    memcpy(&pin_enter_arguments, arguments, sizeof(struct screen_pin_enter_arguments));
}

static void pin_enter_draw(struct nokia_ui *ui)
{
    display_blit_image(ui, 0, 0, &pen);

    int menux = DISPLAY_WIDTH/2 - fonts_get_pixel_width(font_3310_small_bold, pin_enter_arguments.button_text) / 2;
    int menuy = DISPLAY_HEIGHT - fonts_get_pixel_height(font_3310_small_bold);
    fonts_draw_text(ui, font_3310_small_bold, pin_enter_arguments.button_text, menux, menuy);

    //draw enter pin text
    fonts_draw_text(ui, font_3310_small_bold, pin_enter_arguments.line1_text, 0, 8);
    fonts_draw_text(ui, font_3310_small_bold, pin_enter_arguments.line2_text, 0, 8 + fonts_get_pixel_height(font_3310_small_bold));

    //draw stars
    for(int i = 0; i < *pin_enter_arguments.entered_text_len; i++)
        fonts_draw_text(ui, font_3310_small_bold, "*", i*fonts_get_pixel_width(font_3310_small_bold, "*"), 8 + 2*fonts_get_pixel_height(font_3310_small_bold));
}

static void pin_enter_handle_input(struct nokia_ui *ui, int keys)
{
    char chr;

    switch(keys)
    {
        case '0' ... '9':
            if((*pin_enter_arguments.entered_text_len) < pin_enter_arguments.max_input_len)
            {
                chr = (char)keys;
                pin_enter_arguments.input_buffer[(*pin_enter_arguments.entered_text_len)] = chr;
                (*pin_enter_arguments.entered_text_len)++;
            }
            break;

        case NOKIA_KEY_BACKSPACE:
            if((*pin_enter_arguments.entered_text_len) > 0)
            {
                pin_enter_arguments.input_buffer[(*pin_enter_arguments.entered_text_len)-1] = 0;
                (*pin_enter_arguments.entered_text_len)--;
            }
            break;

        case NOKIA_KEY_ENTER:
            if((*pin_enter_arguments.entered_text_len) >= pin_enter_arguments.min_input_len)
                screen_set_current(ui, pin_enter_arguments.success_return_screen, pin_enter_arguments.success_return_screen_arguments);
            break;
    }
}

struct screen screen_pin_enter = {
    .enter_callback = pin_enter_enter,
    .leave_callback = screen_leave_null,
    .draw_callback = pin_enter_draw,
    .handle_input = pin_enter_handle_input,
    .handle_baseband = screen_handle_baseband_null,
};
