#include "ui.h"

#include "display.h"
#include "keyboard.h"
#include "screen.h"
#include "animations.h"
#include "uictl.h"
#include "ui_event.h"

#include <SDL.h>

#define DISPLAY_SCALE 8

struct sdl_display {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *fb_texture;
    uint32_t *pixels;
};

static void sdl_ui_swap(struct nokia_ui* ui);
static int sdl_handle_special_keys(int scancode);

int ui_init(struct nokia_ui* ui)
{
    SDL_Init(SDL_INIT_VIDEO);

    ui->display_entity.ui_subsys = malloc(sizeof(struct sdl_display));
    struct sdl_display *sdl_display = ui->display_entity.ui_subsys;

    sdl_display->window = SDL_CreateWindow(
        "Nokia",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        DISPLAY_WIDTH * DISPLAY_SCALE,
        DISPLAY_HEIGHT * DISPLAY_SCALE,
        0
    );

    SDL_StartTextInput();

    sdl_display->renderer = SDL_CreateRenderer(sdl_display->window, -1, 0 /*SDL_RENDERER_ACCELERATED*/);
    sdl_display->fb_texture = SDL_CreateTexture(sdl_display->renderer, SDL_PIXELFORMAT_RGBX8888, SDL_TEXTUREACCESS_STREAMING, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    sdl_display->pixels = malloc(sizeof(uint32_t) * DISPLAY_HEIGHT * DISPLAY_WIDTH);

    return 0;
}

void ui_exit(struct nokia_ui* ui)
{
    struct sdl_display *sdl_display = ui->display_entity.ui_subsys;

    if(sdl_display->fb_texture)
        SDL_DestroyTexture(sdl_display->fb_texture);

    if(sdl_display->renderer)
        SDL_DestroyRenderer(sdl_display->renderer);

    if(sdl_display->window)
        SDL_DestroyWindow(sdl_display->window);

    if(sdl_display->pixels)
        free(sdl_display->pixels);

    if(sdl_display)
        free(sdl_display);
}

void ui_display_clear(struct nokia_ui* ui)
{
    struct sdl_display *sdl_display = ui->display_entity.ui_subsys;
    memset(sdl_display->pixels, 0xFF, sizeof(uint32_t) * DISPLAY_HEIGHT * DISPLAY_WIDTH);
}

void ui_display_set_pixel(struct nokia_ui* ui, int x, int y)
{
    struct sdl_display *sdl_display = ui->display_entity.ui_subsys;

    if(x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT || x < 0 || y < 0)
        return;

    sdl_display->pixels[y*DISPLAY_WIDTH+x] = 0;
}


void ui_display_clear_pixel(struct nokia_ui* ui, int x, int y)
{
    struct sdl_display *sdl_display = ui->display_entity.ui_subsys;

    if(x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT || x < 0 || y < 0)
        return;

    sdl_display->pixels[y*DISPLAY_WIDTH+x] = 0xFFFFFFFF;
}

void ui_display_invert_pixel(struct nokia_ui* ui, int x, int y)
{
    struct sdl_display *sdl_display = ui->display_entity.ui_subsys;

    if(x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT || x < 0 || y < 0)
        return;

    sdl_display->pixels[y*DISPLAY_WIDTH+x] ^= 0xFFFFFFFF;
}

void ui_render(struct nokia_ui* ui)
{
    int chr;
    SDL_Event e;
    struct msgb *msg;

    if (SDL_PollEvent(&e))
    {
        switch(e.type)
        {
            case SDL_QUIT:
                uictl_send_mobile_shutdown(ui, 0);
                break;

            case SDL_TEXTINPUT:
                screen_handle_input(ui, e.text.text[0]);
                break;

            case SDL_KEYDOWN:
                chr = e.key.keysym.sym;
                if(chr < 0x20 || chr > 0x126)
                    screen_handle_input(ui, sdl_handle_special_keys(e.key.keysym.sym));
                break;
        }
    }

    while((msg = msgb_dequeue(&ui->ui_entity.event_queue))) {
        struct ui_event_msg* uiev = (struct ui_event_msg *)msg->data;
        screen_handle_baseband(ui, uiev->msg_type, &uiev->u, NULL);
        msgb_free(msg);
    }

    ui_display_clear(ui);
    screen_draw(ui);
    animation_play(ui);
    sdl_ui_swap(ui);

    osmo_timer_schedule(&ui->ui_entity.render_timer, 0, (1*1000*1000)/UI_FPS);
}

static void sdl_ui_swap(struct nokia_ui* ui)
{
    struct sdl_display *sdl_display = ui->display_entity.ui_subsys;

    SDL_Rect dest = {
        .x = 0,
        .y = 0,
        .w = DISPLAY_WIDTH * DISPLAY_SCALE,
        .h = DISPLAY_HEIGHT * DISPLAY_SCALE,
    };

    SDL_UpdateTexture(sdl_display->fb_texture, NULL, sdl_display->pixels, DISPLAY_WIDTH * sizeof(uint32_t));

    SDL_RenderClear(sdl_display->renderer);
    SDL_RenderCopy(sdl_display->renderer, sdl_display->fb_texture , NULL, &dest);
    SDL_RenderPresent(sdl_display->renderer);
}

static int sdl_handle_special_keys(int scancode)
{
    switch(scancode) {
        case SDLK_BACKSPACE:
            return NOKIA_KEY_BACKSPACE;
        case SDLK_UP:
            return NOKIA_KEY_UP;
        case SDLK_DOWN:
            return NOKIA_KEY_DOWN;
        case SDLK_LEFT:
            return NOKIA_KEY_LEFT;
        case SDLK_RIGHT:
            return NOKIA_KEY_RIGHT;
        case SDLK_ESCAPE:
            return NOKIA_KEY_ESC;
        case SDLK_RETURN:
            return NOKIA_KEY_ENTER;
        default:
            return 0;
    }
}
