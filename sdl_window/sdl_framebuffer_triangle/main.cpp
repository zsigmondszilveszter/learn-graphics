#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstdint>
#include <iostream>
#include <chrono>
#include <cmath>
#include <thread>
#include <algorithm>

#include "cli_args_szilv.hpp"
#include "base_geometry.hpp"
#include "2D_triangle.hpp"
#include "2D_line_drawer.hpp"


static uint64_t loop_count = 0;
static auto prev_time = std::chrono::steady_clock::now();
static uint64_t prev_loop_count = 0;

/**
 *
 */
uint32_t fps_counter_callback(void* userdata, SDL_TimerID timerID, uint32_t interval) {
    auto now = std::chrono::steady_clock::now();

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
szilv::SquareDefinition defineTheSquareContainingTheTriangles(szilv::Triangle2D * tr1, szilv::Triangle2D * tr2) {
    // find a square the two triangle fit inside
    auto prmt1 = tr1->getPrimitive();
    auto prmt2 = tr2->getPrimitive();
    return {
        (int32_t) std::min({prmt1.p1.x, prmt1.p2.x, prmt1.p3.x, prmt2.p1.x, prmt2.p2.x, prmt2.p3.x}),
        (int32_t) std::min({prmt1.p1.y, prmt1.p2.y, prmt1.p3.y, prmt2.p1.y, prmt2.p2.y, prmt2.p3.y}),
        (int32_t) std::max({prmt1.p1.x, prmt1.p2.x, prmt1.p3.x, prmt2.p1.x, prmt2.p2.x, prmt2.p3.x}),
        (int32_t) std::max({prmt1.p1.y, prmt1.p2.y, prmt1.p3.y, prmt2.p1.y, prmt2.p2.y, prmt2.p3.y}),
    };
}

/**
 *
 */
void calculateTheTrianglePositionAndSize(
        szilv::Triangle2D * triangle,
        uint32_t window_width,
        uint32_t window_height,
        uint32_t desired_side_length) {

    uint32_t smaller_side = std::min(window_width, window_height);
    uint32_t constrainted_radius = smaller_side / 2;
    uint32_t desired_radius = (double)desired_side_length / sqrt(3);
    uint32_t new_radius = std::min(constrainted_radius, desired_radius);
    uint32_t new_side_length = new_radius * sqrt(3);
    szilv::TrianglePrimitive trg_primitive = triangle->getPrimitive();
    double old_side_length = szilv::Triangle2D::distance(trg_primitive.p1, trg_primitive.p2);

    // scale the triangle
    double scale = (double)new_side_length / old_side_length;
    szilv::Vertex old_center = triangle->getCenter();
    triangle->setPrimitive({
            { old_center.x + scale * (trg_primitive.p1.x - old_center.x), old_center.y + scale * (trg_primitive.p1.y - old_center.y), 0 },
            { old_center.x + scale * (trg_primitive.p2.x - old_center.x), old_center.y + scale * (trg_primitive.p2.y - old_center.y), 0 },
            { old_center.x + scale * (trg_primitive.p3.x - old_center.x), old_center.y + scale * (trg_primitive.p3.y - old_center.y), 0 },
            });

    // set the center
    szilv::Vertex new_center = {(double)window_width / 2, (double)window_height / 2, 0};
    triangle->translateToNewCenter(new_center);
}

/**
 *
 */
int main(int argc, char *argv[]) {
    uint32_t default_cpus = std::max(2U, std::thread::hardware_concurrency());
    // argument parser
    szcl::CliArgsSzilv cliArgs("sdl_framebuffer_triangle", "This program draws a Triangle using SDL3 for window creation and software rendering. "
            "Theoretically it supports all the platforms whatever SDL3 supports.\nAuthor Szilveszter Zsigmond.");
    try {
        cliArgs.addOptionInteger("s,triangle-side-size", "The size of the triangle side.", 400);
        cliArgs.addOptionInteger("w,parallel-draw-workers", "The number of parallel draw workers. Default is the number of available CPUs.", default_cpus);
        cliArgs.addOptionInteger("buffer-slice", "The size of buffer slice we are pushing to one draw worker once.", 10);
        cliArgs.addOptionHelp("h,help", "Prints this help message.");
        cliArgs.parseArguments(argc, argv);
    } catch (szcl::CliArgsSzilvException& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }
    if (cliArgs.isHelp()) {
        std::cout << cliArgs.getHelpDisplay() << std::endl;
        return 0;
    }
    uint32_t nr_of_draw_workers = cliArgs.has("w") ? cliArgs.getOptionInteger("w") : default_cpus;
    uint32_t buffer_slice = cliArgs.has("buffer-slice") ? cliArgs.getOptionInteger("buffer-slice") : 10;
    const uint32_t trg_side = cliArgs.has("s") ? cliArgs.getOptionInteger("s") : 400;

    // -----------------------
    // SDL
    // -----------------------
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow(
        "Szilv triangle software renderer with SDL3",
        800, 600, // this is a hint only. the final window size depends on things (for example tiling window managers ignore this)
        SDL_WINDOW_RESIZABLE
    );
    int32_t w, h;
    SDL_GetWindowSize(window, &w, &h);
    SDL_Renderer *ren = SDL_CreateRenderer(window, "software");
    SDL_Event e;
    SDL_Texture *tex = SDL_CreateTexture(
            ren,
            SDL_PIXELFORMAT_XRGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            w, h
            );

    uint32_t fps_report_interval = 1000; // ms
    SDL_TimerID timerID = SDL_AddTimer(fps_report_interval, fps_counter_callback, nullptr); 

    // -----------------------
    // Triangle
    // -----------------------
    std::vector<szilv::LineDrawer2D *> workers;
    // current
    // initial position and orientation of the triangle
    double trg_offset_x = 0;
    double trg_offset_y = 0;
    const double sin60 = sin(60 * M_PI / 180);
    const double cos60 = cos(60 * M_PI / 180);
    double trg_height = trg_side * sin60;

    szilv::Triangle2D triangle = szilv::Triangle2D(
                    { trg_offset_x + trg_side * cos60,      trg_offset_y,               0 },
                    { trg_offset_x,                         trg_offset_y + trg_height,  0 },
                    { trg_offset_x + trg_side,              trg_offset_y + trg_height,  0 }
                    );
    szilv::Triangle2D * new_triangle = &triangle;
    calculateTheTrianglePositionAndSize(new_triangle, w, h, trg_side);

    // old
    szilv::Triangle2D triangle2 = szilv::Triangle2D({0,0}, {0,0}, {0,0});
    szilv::Triangle2D * old_triangle = &triangle2;

    // start worker threads
    for ( uint32_t i = 0; i < nr_of_draw_workers; i++) {
        workers.push_back(new szilv::LineDrawer2D(i, 0, 0));
    }


    auto prev_timestamp = std::chrono::steady_clock::now();
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
                        // check if the triangle sides should be recalculated
                        calculateTheTrianglePositionAndSize(new_triangle, w, h, trg_side);
                    }
                    break;
            }
        }

        // then draw something
        auto now = std::chrono::steady_clock::now();
        auto elapsed = now - prev_timestamp;
        double angle = std::chrono::duration_cast<std::chrono::duration<double>>(elapsed).count();

        void *pixels;
        int32_t pitch;
        SDL_LockTexture(tex, NULL, &pixels, &pitch);
        int32_t *fb = (int32_t *)pixels;

        // rotate the Triangle
        new_triangle->rotateAroundTheCenter(angle);

        szilv::SquareDefinition squareCoordinates = defineTheSquareContainingTheTriangles(new_triangle, old_triangle);
        // distribute slices of the big 2D square, the triangle is inside, between worker threads
        uint32_t slice = 0;
        for (int32_t y=squareCoordinates.y1; y <= squareCoordinates.y2; y+=buffer_slice) {
            szilv::SquareDefinition square_slice = {
                squareCoordinates.x1, y, 
                squareCoordinates.x2, std::min(y + (int32_t)buffer_slice, squareCoordinates.y2)
            }; 
            auto isInside = [new_triangle](szilv::Vertex point) -> bool {
                return new_triangle->pointInTriangle(point);
            };

            szilv::DrawWork work = {
                0x4285f4,      // triangle color
                0x0,    // background color
                (void*)new_triangle, isInside,
                square_slice, fb,
                (uint32_t)w, (uint32_t)h
            };
            auto worker = workers[slice % nr_of_draw_workers];
            worker->addWorkBlocking(work);
            slice++;
        }

        // update the old Triangle
        old_triangle->setPrimitive(new_triangle->getPrimitive());

        SDL_UnlockTexture(tex);
        SDL_RenderTexture(ren, tex, NULL, NULL);
        SDL_RenderPresent(ren);

        loop_count++;
        prev_timestamp = now;
    }

    // join worker threads
    while (workers.size()) {
        delete workers.back(); // the destructor calls the thread join
        workers.pop_back();
    }
    SDL_RemoveTimer(timerID);
    SDL_Quit();
    return 0;
}
