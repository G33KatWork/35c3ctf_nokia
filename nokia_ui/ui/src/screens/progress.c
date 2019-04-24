#include "screens/progress.h"

#include <string.h>
#include <assert.h>

#include "display.h"
#include "keyboard.h"
#include "fonts.h"
#include "screens/menu.h"

#include "generated/font_small_bold.h"
#include "generated/nokia_images.h"

static void progress_screen_draw(struct nokia_ui *ui);

static struct screen_progress_arguments progress_arguments;
static int i = 2;

static void progress_screen_enter(struct nokia_ui *ui, void* arguments)
{
    assert(arguments);
    memcpy(&progress_arguments, arguments, sizeof(struct screen_progress_arguments));
}

static void progress_screen_leave(struct nokia_ui *ui)
{
}

static void progress_screen_draw(struct nokia_ui *ui)
{
    display_blit_image(ui, 2+i, 7, &progress_inner);
    display_blit_image(ui, i-DISPLAY_WIDTH+6, 7, &progress_inner);

    display_draw_rectangle(ui, 0, 5, 83, 12, 1);
    display_draw_rectangle(ui, 1, 6, 82, 11, 0);

    i++;
    if(i >= DISPLAY_WIDTH-4)
        i = 0;

    fonts_draw_text(ui, font_3310_small_bold, progress_arguments.text_line1, 0, 17);
    fonts_draw_text(ui, font_3310_small_bold, progress_arguments.text_line2, 0, 17 + fonts_get_pixel_height(font_3310_small_bold) + 2);

    if(progress_arguments.text_button)
    {
        int buttonx = DISPLAY_WIDTH/2 - fonts_get_pixel_width(font_3310_small_bold, progress_arguments.text_button) / 2;
        int buttony = DISPLAY_HEIGHT - fonts_get_pixel_height(font_3310_small_bold);
        fonts_draw_text(ui, font_3310_small_bold, progress_arguments.text_button, buttonx, buttony);
    }
}

static void progress_screen_handle_input(struct nokia_ui *ui, int keys)
{
    switch(keys)
    {
        //case 27:    //ESC
        //    screen_set_current(ui, progress_arguments.return_screen, progress_arguments.return_screen_arguments);
        //    break;

        case NOKIA_KEY_ENTER:
            if(progress_arguments.text_button)
                screen_set_current(ui, progress_arguments.button_screen, progress_arguments.button_screen_arguments);
            break;
    }
}

static void progress_screen_handle_baseband(struct nokia_ui *ui, int event, void* data1, void* data2)
{
    if(progress_arguments.handle_baseband_cb)
        progress_arguments.handle_baseband_cb(ui, event, data1, data2);
}

struct screen screen_progress = {
    .enter_callback = progress_screen_enter,
    .leave_callback = progress_screen_leave,
    .draw_callback = progress_screen_draw,
    .handle_input = progress_screen_handle_input,
    .handle_baseband = progress_screen_handle_baseband,
};
