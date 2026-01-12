#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstdint>
#include <iostream>
#include <chrono>
#include <cmath>


static uint64_t loop_count = 0;
static std::chrono::time_point prev_time = std::chrono::steady_clock::now();
static uint64_t prev_loop_count = 0;

/**
 *
 */
uint32_t fps_counter_callback(void* userdata, SDL_TimerID timerID, uint32_t interval) {
    std::chrono::time_point now = std::chrono::steady_clock::now();

    auto elapsed = now - prev_time;
    uint64_t frames = loop_count - prev_loop_count;
    // Convert elapsed time to a double representing seconds
    double seconds_passed = std::chrono::duration_cast<std::chrono::duration<double>>(elapsed).count();
    double fps = frames / seconds_passed;
    std::clog << "FPS: " << std::round(fps) << "\r" << std::flush;

    prev_loop_count = loop_count;
    prev_time = now;

    return interval; // Return the same interval to stay on a loop
}

/**
 *
 */
int main(int argc, char *argv[]) {
    int w, h;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow(
        "Szilv software renderer 1 with SDL3",
        800, 600, // hint only
        SDL_WINDOW_RESIZABLE
    );
    SDL_GetWindowSize(window, &w, &h);

    SDL_Renderer *ren = SDL_CreateRenderer(window, "software");

    uint32_t fps_report_interval = 1000; // ms

    SDL_Event e;
    SDL_Texture *tex = SDL_CreateTexture(
            ren,
            SDL_PIXELFORMAT_XRGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            w, h
            );

    SDL_TimerID timerID = SDL_AddTimer(fps_report_interval, fps_counter_callback, nullptr); // 2000ms = 2 seconds

    int32_t running = 1;
    while (running) {
        // first, handle all the pending events
        while(SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_EVENT_QUIT: 
                    {
                        running = 0;
                    }
                    break;

                case SDL_EVENT_WINDOW_RESIZED: 
                    {
                        SDL_GetWindowSize(window, &w, &h);
                        std::clog << "Window size: " << w << ", " << h << std::endl << std::flush;
                        // destroy previous texture 
                        SDL_DestroyTexture(tex);
                        // create a new one
                        tex = SDL_CreateTexture(
                                ren,
                                SDL_PIXELFORMAT_XRGB8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                w, h
                                );
                    }
                    break;
            }
        }

        // then draw something
        void *pixels;
        int32_t pitch;
        SDL_LockTexture(tex, NULL, &pixels, &pitch);
        uint32_t *fb = (uint32_t *)pixels;

        std::chrono::time_point now = std::chrono::steady_clock::now();
        double t = now.time_since_epoch().count() / 1000000000.0;
        double f = 2.0; // freq

        uint32_t color = 0xFF000000 
                    | (uint64_t)(127 * sin(t * f) + 128) << 16
                    | (uint64_t)(127 * sin(t * f + 2 * M_PI / 3) + 128) << 8
                    | (uint64_t)(127 * sin(t * f + 4 * M_PI / 3) + 128);

        for (uint32_t x = 0; x < w; x++) {
            for (uint32_t y = 0; y< h; y++ ) {
                fb[x + y * w] = color;
            }
        }

        SDL_UnlockTexture(tex);
        SDL_RenderTexture(ren, tex, NULL, NULL);
        SDL_RenderPresent(ren);

        loop_count++;
    }

    SDL_RemoveTimer(timerID);
    SDL_Quit();
    return 0;
}
