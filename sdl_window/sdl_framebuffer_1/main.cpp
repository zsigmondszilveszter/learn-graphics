#include <SDL3/SDL.h>
#include <cstdint>
#include <iostream>

void draw_square(uint32_t *buffer, uint32_t offset_x, uint32_t offset_y, 
        uint32_t dimension, uint32_t color, uint32_t buffer_width) {
    uint32_t buf_offset = offset_y * buffer_width + offset_x;
    for (uint32_t i=buf_offset; i < buf_offset + dimension * buffer_width; i += buffer_width) {
        for (uint32_t j=0; j < dimension; j++) {
            buffer[i+j] = color;
        }
    }
}

int32_t main(int32_t argc, char ** args) {
    uint32_t off_x = 100, off_y = 50;
    uint32_t square_dimension = 70;
    uint32_t color_blue = 0x176BEF; // google blue
    uint32_t color_green = 0xFF3E30; // google green
    uint32_t color_yellow = 0xF7B529; // google yellow
    uint32_t color_red = 0x179C52; // google red

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow(
        "Szilv software renderer 1 with SDL3",
        800, 600, // hint only
        SDL_WINDOW_RESIZABLE
    );
    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    SDL_Renderer *ren = SDL_CreateRenderer(window, NULL);

    int32_t running = 1;
    bool draw = true;
    SDL_Event e;
    while (running) {
        if (draw) {
            SDL_GetWindowSize(window, &w, &h);
            std::clog << "Window size w=" << w << ", h=" << h << std::endl;

            SDL_Texture *tex = SDL_CreateTexture(
                    ren,
                    SDL_PIXELFORMAT_XRGB8888,
                    SDL_TEXTUREACCESS_STREAMING,
                    w, h
                    );

            void *pixels;
            int32_t pitch;

            SDL_LockTexture(tex, NULL, &pixels, &pitch);

            uint32_t *fb = (uint32_t *)pixels;

            // draw something
            draw_square(fb, off_x+square_dimension*0, off_y, square_dimension, color_blue,  w);
            draw_square(fb, off_x+square_dimension*1, off_y, square_dimension, color_green, w);
            draw_square(fb, off_x+square_dimension*2, off_y, square_dimension, color_yellow,w);
            draw_square(fb, off_x+square_dimension*3, off_y, square_dimension, color_red,   w);

            SDL_UnlockTexture(tex);

            SDL_RenderTexture(ren, tex, NULL, NULL);
            SDL_RenderPresent(ren);

            draw = false;
        }

        SDL_WaitEventTimeout(&e, 100);
        switch (e.type) {
            case SDL_EVENT_QUIT: 
                {
                    running = 0;
                }
                break;

            case SDL_EVENT_WINDOW_RESIZED: 
                {
                    draw = true;
                }
                break;
        }
    }

    SDL_Quit();
    return 0;
}
