#include <iostream> 
#include <csignal>
#include <chrono>
#include <cmath>

#include <cxxopts.hpp>
#include <mouse_event_reader.hpp>
#include <drm_util.h>
#include "base_geometry.h"
#include "triangle.h"
#include "line_drawer.h"
#include "fps_digits.h"


#define BUFFER_SLICE 10
#define BUFFER_UPPER_LIMIT 1000
#define NANO_TO_SEC_CONV 1000000000L


const uint32_t color_blue     = 0x4285f4; // google blue
const uint32_t color_green    = 0x0F9D58; // google green
const uint32_t color_yellow   = 0xF4B400; // google yellow
const uint32_t color_red      = 0xDB4437; // google red
const uint32_t color_white    = 0xFFFFFF;
const uint32_t color_black    = 0x0;

bool keep_running = true;
bool show_fps = false;
LinuxMouseEvent::MouseEventReader * mouse_event_reader;
const uint32_t nr_of_draw_workers = 15;
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

void draw_triangle(drm_util::modeset_buf * buf, GM::Triangle tr, GM::Triangle old_tr, uint32_t color) {
    uint32_t bg_color = color_black;
    // find a square the triangle is inside
    double new_left_bound = std::min(std::min(tr.getPrimitive().p1.x, tr.getPrimitive().p2.x), tr.getPrimitive().p3.x);
    double new_right_bound = std::max(std::max(tr.getPrimitive().p1.x, tr.getPrimitive().p2.x), tr.getPrimitive().p3.x);
    double new_upper_bound = std::min(std::min(tr.getPrimitive().p1.y, tr.getPrimitive().p2.y), tr.getPrimitive().p3.y);
    double new_lower_bound = std::max(std::max(tr.getPrimitive().p1.y, tr.getPrimitive().p2.y), tr.getPrimitive().p3.y);

    double old_left_bound = std::min(std::min(old_tr.getPrimitive().p1.x, old_tr.getPrimitive().p2.x), old_tr.getPrimitive().p3.x);
    double old_right_bound = std::max(std::max(old_tr.getPrimitive().p1.x, old_tr.getPrimitive().p2.x), old_tr.getPrimitive().p3.x);
    double old_upper_bound = std::min(std::min(old_tr.getPrimitive().p1.y, old_tr.getPrimitive().p2.y), old_tr.getPrimitive().p3.y);
    double old_lower_bound = std::max(std::max(old_tr.getPrimitive().p1.y, old_tr.getPrimitive().p2.y), old_tr.getPrimitive().p3.y);

    int32_t left_bound  = (int32_t)std::min(new_left_bound, old_left_bound);
    int32_t right_bound = (int32_t)std::max(new_right_bound, old_right_bound);
    int32_t upper_bound = (int32_t)std::min(new_upper_bound, old_upper_bound);
    int32_t lower_bound = (int32_t)std::max(new_lower_bound, old_lower_bound);

    // check all the pixels inside the square of these bounds
    uint32_t slice = 0;
    for (int32_t y=upper_bound; y <= lower_bound; y+=BUFFER_SLICE) {
        workers[slice % nr_of_draw_workers]->addWorkBlocking({ left_bound, right_bound, y, 
                std::min(y + BUFFER_SLICE, lower_bound), color, bg_color, buf, 
                SG::Triangle, (void*)&tr});
        slice++;
    }
}

uint32_t fps = 0;
uint32_t max_nr_of_digits = 0;
uint64_t previous_fps_changed_at = get_nanos();
void fps_counter(drm_util::modeset_buf * buf, int64_t t_diff, int64_t time) {
    if (previous_fps_changed_at < time - NANO_TO_SEC_CONV) {
        fps = 1000000000 / t_diff;
        previous_fps_changed_at = time;
    }
    uint32_t nr_of_digits = 0;
    uint32_t tmp = fps;
    while (tmp) {
        char * digit = GM::FpsDigits::getDigit(tmp % 10);
        int32_t left = buf->width - 15 * ((int32_t)nr_of_digits + 1) - 3 * (int32_t)nr_of_digits;
        workers[nr_of_digits % nr_of_draw_workers]->addWorkBlocking(
                { left, left + 15,
                2, 20, 
                color_blue, color_black, buf, SG::Digit, (void*)digit});
        tmp /= 10; 
        nr_of_digits++;
    }
    max_nr_of_digits = std::max(max_nr_of_digits, nr_of_digits);
    while (nr_of_digits < max_nr_of_digits) {
        char * digit = (char*)GM::FpsDigits::blank;
        int32_t left = buf->width - 15 * ((int32_t)nr_of_digits + 1) - 3 * (int32_t)nr_of_digits;
        workers[nr_of_digits % nr_of_draw_workers]->addWorkBlocking(
                { left, left + 15,
                2, 20, 
                color_blue, color_black, buf, SG::Digit, (void*)digit});
        nr_of_digits++;
    }
}

/**
 *
 */
int main(int argc, char **argv) {
    // argument parser
    cxxopts::Options options("Draws a Triangle using Linux DRM library", "This program draws a Triangle using Linux DRM library "
            "using mouse event reader. It can't run under a windowing system like X11/Wayland as it directly opens and writes " 
            "to the given DRI device which is not accessible under X11.\nAuthor Szilveszter Zsigmond.");
    options.add_options()
        ("dri-device", "The dri device path. List devices with \"ls -alh /dev/dri/card*\".", cxxopts::value<std::string>()->default_value("/dev/dri/card0"))
        ("mouse-input-device", "Mouse input device path. List mouse event devices with \"ls -alh /dev/input/by-id\"", cxxopts::value<std::string>()->default_value("/dev/input/event7"))
        ("show-fps", "Show custom built FPS counter in the upper right corner")
        ("h,help", "Prints this help message.");
    auto arguments = options.parse(argc, argv);
    if (arguments.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    // registar signal handler
    signal(SIGINT, sig_handler);

    show_fps = arguments.count("show-fps");

    // initialize the drm device
    std::string drm_card_name = arguments["dri-device"].as<std::string>();
    drmUtil = new drm_util::DrmUtil(drm_card_name.c_str());
    int32_t response = drmUtil->initDrmDev();
    if (response) {
        return response;
    }

    // initialize the MouseEventReader
    std::string input_device_name = arguments["mouse-input-device"].as<std::string>();
    uint32_t max_x = (&drmUtil->mdev->bufs[0])->width;
    uint32_t max_y = (&drmUtil->mdev->bufs[0])->height;
    mouse_event_reader = new LinuxMouseEvent::MouseEventReader(
            input_device_name.c_str(),
            max_x, max_y);
    int32_t response2 = mouse_event_reader->openEventFile();
    if (response2) {
        return response2;
    }

    // draw a triangle and then rotate it
    double trg_offset_x, trg_offset_y, trg_side;
    trg_offset_x = trg_offset_y = trg_side = 400;
    const double sin60 = sin(60 * M_PI / 180);
    const double cos60 = cos(60 * M_PI / 180);
    double trg_height = trg_side * sin60;
    GM::Triangle trg = GM::Triangle(
        { trg_offset_x + trg_side * cos60,      trg_offset_y,               0 },
        { trg_offset_x,                         trg_offset_y + trg_height,  0 },
        { trg_offset_x + trg_side,              trg_offset_y + trg_height,  0 }
    );
    GM::Triangle new_triangle = GM::Triangle(&trg);

    uint32_t max_radius = trg.getRadiusOfTheOuterCircle();

    // start worker threads
    for ( uint32_t i = 0; i < nr_of_draw_workers; i++) {
        workers.push_back(new SG::LineDrawer(i));
    }

    int64_t prev_t = get_nanos();
    drm_util::modeset_buf * buf;

    while (keep_running) {
        int64_t t = get_nanos();
        int64_t t_diff = t - prev_t;
        double angle = (double)(t_diff) * 0.000000001;
        buf = &drmUtil->mdev->bufs[0];

        // copy the old triangle date to the new one
        new_triangle.setPrimitive(trg.getPrimitive());

        // current mouse position
        auto mouse_position = mouse_event_reader->getMousePosition();

        // translate the Triangle
        GM::Vertex new_center = {
            1.0 * std::min(std::max(mouse_position.x, max_radius), buf->width - max_radius),
            1.0 * std::min(std::max(mouse_position.y, max_radius), buf->height - max_radius),
            0
        };
        new_triangle.translateToNewCenter(new_center);

        // rotate the Triangle
        new_triangle.rotateAroundTheCenter(angle);

        // draw the old triangle with black ink and the new with a custom color ink
        draw_triangle(buf, new_triangle, trg, color_white);

        if (show_fps) {
            fps_counter(buf, t_diff, t);
        }

        // wait til all the draws are done
        for (uint32_t i = 0; i < nr_of_draw_workers; i++) {
            workers[i]->blockMainThreadUntilTheQueueIsNotEmpty();
        }

        //
        prev_t = t;
        trg = new_triangle;
    }

    clean_up();

    return 0;
}

