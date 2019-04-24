#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

#define DISPLAY_WIDTH   84
#define DISPLAY_HEIGHT  48

void display_set_pixel(uint32_t *pixels, int x, int y, int val)
{
    if(x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT || x < 0 || y < 0)
        return;

    pixels[y*DISPLAY_WIDTH+x] = !val ? 0 : 0xFFFFFFFF;
}

int main() {
    printf("Hello, World!\n");

    SDL_Window *window;
    SDL_Surface *surface;
    SDL_Renderer *renderer;
    SDL_Texture* fb_texture;
    uint32_t *pixels = NULL;
    int i = 0;

    if(SDL_Init(SDL_INIT_VIDEO)) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "software");

    int numdrivers = SDL_GetNumRenderDrivers (); 
    printf("Render driver count: %u\n", numdrivers);
    for(int i = 0; i < numdrivers; i++) { 
        SDL_RendererInfo drinfo; 
        SDL_GetRenderDriverInfo (0, &drinfo); 

        printf("Driver name (%u): %s\n", i, drinfo.name);
        if(drinfo.flags & SDL_RENDERER_SOFTWARE)
            printf(" the renderer is a software fallback\n");
        if(drinfo.flags & SDL_RENDERER_ACCELERATED)
            printf(" the renderer uses hardware acceleration\n");
        if(drinfo.flags & SDL_RENDERER_PRESENTVSYNC)
            printf(" present is synchronized with the refresh rate\n");
        if(drinfo.flags & SDL_RENDERER_TARGETTEXTURE)
            printf(" the renderer supports rendering to texture\n");

        for(int j = 0; j < drinfo.num_texture_formats; j++)
            printf("Texture format %d: %s\n", j, SDL_GetPixelFormatName(drinfo.texture_formats[j]));
    }

    window = SDL_CreateWindow(
        "Nokia",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        DISPLAY_WIDTH,
        DISPLAY_HEIGHT,
        0 /*SDL_WINDOW_FULLSCREEN*/
    );
    if(!window) {
        printf("Failed to create window: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_StartTextInput();

    
    //surface = SDL_GetWindowSurface(window);
    //if(!surface) {
    //    printf("Failed to create surface: %s\n", SDL_GetError());
    //    exit(1);
    //}

    //renderer = SDL_CreateSoftwareRenderer(surface);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE /*SDL_RENDERER_ACCELERATED*/);
    if(!renderer) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        exit(1);
    }

    fb_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    if(!fb_texture) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        exit(1);
    }

    pixels = malloc(sizeof(uint32_t) * DISPLAY_HEIGHT * DISPLAY_WIDTH);

    while(1) {
        SDL_Event e;
        if (SDL_PollEvent(&e))
        {
            switch(e.type)
            {
                case SDL_QUIT:
                    goto exit;

                case SDL_TEXTINPUT:
                    //screen_handle_input(ui, e.text.text[0]);
                    printf("SDL_TEXTINPUT: 0x%x\n", e.text.text[0]);
                    break;

                case SDL_KEYDOWN:
                    //chr = e.key.keysym.sym;
                    //if(chr < 0x20 || chr > 0x126)
                    //    screen_handle_input(ui, chr);
                    printf("SDL_KEYDOWN: 0x%x\n", e.key.keysym.sym);
                    break;
            }
        }

        // Draw
        memset(pixels, i, sizeof(uint32_t) * DISPLAY_HEIGHT * DISPLAY_WIDTH);

        for(int y = 0; y < DISPLAY_HEIGHT; y++)
            for(int x = 0; x < DISPLAY_WIDTH; x++)
                display_set_pixel(pixels, x, y, i);

        i ^= 1;

        // Update
        SDL_Rect dest = {
            .x = 0,
            .y = 0,
            .w = DISPLAY_WIDTH,
            .h = DISPLAY_HEIGHT,
        };

        SDL_UpdateTexture(fb_texture, NULL, pixels, DISPLAY_WIDTH * sizeof(uint32_t));

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, fb_texture , NULL, &dest);
        SDL_RenderPresent(renderer);
    }

exit:
    SDL_DestroyTexture(fb_texture);
    free(pixels);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}
