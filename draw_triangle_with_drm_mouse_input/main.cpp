#include <iostream> 
#include <csignal>
#include <cmath>
#include <vector>
#include <algorithm>
#include <optional>

#include <cxxopts_wrapper.hpp>
#include <mouse_event_reader.hpp>
#include <drm_util.h>
#include "base_geometry.h"
#include "triangle.h"
#include "line_drawer.h"
#include "fps_digits.h"
#include "tools.hpp"


#define NANO_TO_SEC_CONV 1000000000L


const uint32_t color_blue     = 0x4285f4; // google blue
const uint32_t color_green    = 0x0F9D58; // google green
const uint32_t color_yellow   = 0xF4B400; // google yellow
const uint32_t color_red      = 0xDB4437; // google red
const uint32_t color_white    = 0xFFFFFF;
const uint32_t color_black    = 0x0;

bool keep_running = true;
bool show_fps = false;
bool double_buffering = false;
uint32_t nr_of_draw_workers = 2U; // the last fallback
uint32_t buffer_slice = 10;
LinuxMouseEvent::MouseEventReader * mouse_event_reader;
std::vector<SG::LineDrawer *> workers;
drm_util::DrmUtil * drmUtil;


/**
 *
 */
void clean_up() {
    // join worker threads
    while (workers.size()) {
        delete workers.back(); // the destructor calls the thread join
        workers.pop_back();
    }
    delete mouse_event_reader;
    delete drmUtil;
}

/**
 *
 */
void sig_handler(int signo) {
    if (signo == SIGINT) {
        std::cerr << " - Received SIGINT, cleaning up." << std::endl;
        keep_running = false;

        clean_up();

        exit(0);
    }
}

static int64_t get_nanos(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (int64_t)ts.tv_sec * NANO_TO_SEC_CONV + ts.tv_nsec;
}

SG::SquareDefinition defineTheSquareContainingTheTriangles(GM::Triangle * tr1, GM::Triangle * tr2) {
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
void distribute_triangle_draws(GM::Triangle * tr, SG::SquareDefinition squareCoordinates, uint32_t color, int32_t * buff) {
    uint32_t bg_color = color_black;
    // distribute slices of the big 2D square, the triangle is inside, between worker threads
    uint32_t slice = 0;
    for (int32_t y=squareCoordinates.y1; y <= squareCoordinates.y2; y+=buffer_slice) {
        SG::SquareDefinition square_slice = {
            squareCoordinates.x1, y, 
            squareCoordinates.x2, std::min(y + (int32_t)buffer_slice, squareCoordinates.y2)
        }; 
        SG::DrawWork work = {
            square_slice,
            color, bg_color, 
            SG::Triangle, (void*)tr
        };
        auto worker = workers[slice % nr_of_draw_workers];
        worker->addWorkBlocking(work, buff);
        slice++;
    }
}

uint32_t previous_nr_of_digits = 0;
SG::SquareDefinition fps_counter(drm_util::modeset_buf * buf, int64_t t_diff, int64_t time, SG::LineDrawer * worker) {
    uint32_t fps = NANO_TO_SEC_CONV / t_diff;
    uint32_t nr_of_digits = 0;
    uint32_t tmp = fps;
    while (tmp) {
        nr_of_digits++;
        tmp /= 10;
    }

    SG::SquareDefinition squareToFitEveryNumberIn = {0, 2, (int32_t)(buf->width), 20};

    uint32_t this_round_max = std::max(previous_nr_of_digits, nr_of_digits);
    for (uint32_t i = 0; i < this_round_max; i++) {
        char * digit = fps 
            ? GM::FpsDigits::getDigit(fps % 10)
            : (char*)GM::FpsDigits::blank;
        int32_t left = buf->width - 15 * (i + 1) - 3 * i;
        squareToFitEveryNumberIn.x1 = left;
        SG::SquareDefinition square_slice = {
            left, 2, 
            left + 15, 20
        }; 
        SG::DrawWork work = {
            square_slice,
            color_blue, color_black, 
            SG::Digit, (void*)digit
        };
        worker->addWorkBlocking(work);
        fps /= 10; 
    }
    previous_nr_of_digits = this_round_max;
    return squareToFitEveryNumberIn;
}

/**
 *
 */
int main(int argc, char **argv) {
    // argument parser
    tl::CxxOptsWrapper cxxOptsWrapper("draw_triangle_with_drom_mouse_input", "This program draws a Triangle using Linux DRM library "
            "and moves it by using mouse events. It can't run under a windowing system like X11/Wayland as it directly opens and writes " 
            "to the given DRI device which is not accessible under X11.\nAuthor Szilveszter Zsigmond.");
    cxxOptsWrapper.addOptionString("dri-device", "The dri device path. List devices with \"ls -alh /dev/dri/card*\".", "/dev/dri/card0");
    cxxOptsWrapper.addOptionString("mouse-input-device", "Mouse input device path. List mouse event devices with \"ls -alh /dev/input/by-id\"","/dev/input/event7");
    cxxOptsWrapper.addOptionInteger("s,triangle-side-size", "The size of the triangle side. The default is 400.");
    cxxOptsWrapper.addOptionInteger("w,parallel-draw-workers", "The number of parallel draw workers. Default is the number of available CPUs.");
    cxxOptsWrapper.addOptionInteger("buffer-slice", "The size of buffer slice we are pushing to one draw worker once. Default is 10.");
    cxxOptsWrapper.addOptionBoolean("double-buffering", "Use double buffer from the DRM library");
    cxxOptsWrapper.addOptionBoolean("show-fps", "Show custom built FPS counter in the upper right corner");
    cxxOptsWrapper.addOptionHelp("Prints this help message.");
    cxxOptsWrapper.parseArguments(argc, argv);
    if (cxxOptsWrapper.count("help")) {
        std::cout << cxxOptsWrapper.getHelp() << std::endl;
        return 0;
    }

    // registar signal handler
    signal(SIGINT, sig_handler);

    show_fps = cxxOptsWrapper.count("show-fps");
    nr_of_draw_workers = cxxOptsWrapper.count("w") ? cxxOptsWrapper.getOptionInteger("w") : std::max(2U, tl::Tools::nr_of_cpus());
    double_buffering = cxxOptsWrapper.count("double-buffering");
    buffer_slice = cxxOptsWrapper.count("bl") ? cxxOptsWrapper.getOptionInteger("bl") : buffer_slice;

    // initialize the drm device
    std::string drm_card_name = cxxOptsWrapper.getOptionString("dri-device");
    drmUtil = new drm_util::DrmUtil(drm_card_name.c_str());
    int32_t response = drmUtil->initDrmDev();
    if (response) {
        return response;
    }
    drm_util::modeset_buf * buf = &drmUtil->mdev->bufs[0];

    // initialize the MouseEventReader
    std::string input_device_name = cxxOptsWrapper.getOptionString("mouse-input-device");
    uint32_t max_x = (&drmUtil->mdev->bufs[0])->width;
    uint32_t max_y = (&drmUtil->mdev->bufs[0])->height;
    mouse_event_reader = new LinuxMouseEvent::MouseEventReader(
            input_device_name.c_str(),
            max_x, max_y);
    int32_t response2 = mouse_event_reader->openEventFile();
    if (response2) {
        return response2;
    }

    // initial position and orientation of the triangle
    const uint32_t trg_side = cxxOptsWrapper.count("s") ? cxxOptsWrapper.getOptionInteger("s") : 400;
    double trg_offset_x = 0;
    double trg_offset_y = 0;
    const double sin60 = sin(60 * M_PI / 180);
    const double cos60 = cos(60 * M_PI / 180);
    double trg_height = trg_side * sin60;

    // current
    GM::Triangle triangle = GM::Triangle(
                    { trg_offset_x + trg_side * cos60,      trg_offset_y,               0 },
                    { trg_offset_x,                         trg_offset_y + trg_height,  0 },
                    { trg_offset_x + trg_side,              trg_offset_y + trg_height,  0 }
                    );
    GM::Triangle * new_triangle = &triangle;

    // old
    uint32_t nr_of_triangle_buffers = double_buffering ? 2 : 1;
    std::vector<GM::Triangle> old_triangles(nr_of_triangle_buffers, new GM::Triangle(
                { trg_offset_x + trg_side * cos60,      trg_offset_y,               0 },
                { trg_offset_x,                         trg_offset_y + trg_height,  0 },
                { trg_offset_x + trg_side,              trg_offset_y + trg_height,  0 }
                )
            );
    GM::Triangle * old_triangle = nullptr;

    uint32_t max_radius = new_triangle->getRadiusOfTheOuterCircle();

    // start worker threads
    for ( uint32_t i = 0; i < nr_of_draw_workers; i++) {
        workers.push_back(new SG::LineDrawer(i, buf->width, buf->height));
    }

    int64_t prev_t = get_nanos();
    uint64_t counter = 0;
    std::optional<SG::SquareDefinition> fps_updated = std::nullopt;
    auto fps_draw_worker = workers[nr_of_draw_workers - 1];
    uint64_t previous_fps_changed_at = get_nanos();
    bool keep_fps_one_more_frame = true;

    while (keep_running) {
        int64_t t = get_nanos();
        int64_t t_diff = t - prev_t;
        double angle = (double)(t_diff) * 0.000000001;

        uint32_t buf_idx = double_buffering ? drmUtil->mdev->front_buf ^ 1 : 0;
        buf = &drmUtil->mdev->bufs[buf_idx];
        old_triangle = &old_triangles[buf_idx];

        // current mouse position
        auto mouse_position = mouse_event_reader->getMousePosition();
        GM::Vertex new_center = {
            1.0 * std::min(std::max(mouse_position.x, max_radius), buf->width - max_radius),
            1.0 * std::min(std::max(mouse_position.y, max_radius), buf->height - max_radius),
            0
        };

        // translate the Triangle
        new_triangle->translateToNewCenter(new_center);

        // rotate the Triangle
        new_triangle->rotateAroundTheCenter(angle);

        SG::SquareDefinition squareCoordinates = defineTheSquareContainingTheTriangles(new_triangle, old_triangle);
        // draw the old triangle with black ink and the new with a custom color ink
        distribute_triangle_draws(new_triangle, squareCoordinates, color_white, buf->map);

        // update the old Triangle
        old_triangle->setPrimitive(new_triangle->getPrimitive());

        if (show_fps && previous_fps_changed_at < t - NANO_TO_SEC_CONV) {
            fps_updated = std::optional<SG::SquareDefinition>{fps_counter(buf, t_diff, t, fps_draw_worker)};
            previous_fps_changed_at = t;
        }

        // the fps digits from draw workers' buffers into the DRM buffer
        if (fps_updated.has_value()) {
            fps_draw_worker->memCopySquareInto(fps_updated.value(), buf->map);
            if (!keep_fps_one_more_frame) {
                fps_updated = std::nullopt;
                keep_fps_one_more_frame = true;
            }
        }

        //
        prev_t = t;

        if (double_buffering) {
            // swap buffers
            drmUtil->swap_buffers();
        }

        counter++;
    }

    clean_up();

    return 0;
}

