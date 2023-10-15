#include <unistd.h>
#include <iostream>
#include <time.h>
#include <math.h>
#include <signal.h>

#include "drm_util.h"
#include "line_drawer.h"
#include "base_geometry.h"

#define BUFFER_SLICE 10
#define BUFFER_UPPER_LIMIT 1000
#define FPS_COUNTER false


uint32_t color_blue = 0x4285f4; // google blue
uint32_t color_green = 0x0F9D58; // google green
uint32_t color_yellow = 0xF4B400; // google yellow
uint32_t color_red = 0xDB4437; // google red
uint32_t color_white = 0xFFFFFF;
uint32_t color_black = 0x0;
bool keep_running = true;
const auto processor_count = std::thread::hardware_concurrency();
std::vector<Szilv::LineDrawer *> workers;

static int64_t get_nanos(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (int64_t)ts.tv_sec * 1000000000L + ts.tv_nsec;
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

    int32_t left_bound = (int32_t)std::min(new_left_bound, old_left_bound);
    int32_t right_bound = (int32_t)std::max(new_right_bound, old_right_bound);
    int32_t upper_bound = (int32_t)std::min(new_upper_bound, old_upper_bound);
    int32_t lower_bound = (int32_t)std::max(new_lower_bound, old_lower_bound);

    // increase the box a little bit to surely cover the previous triangle
    left_bound -= 30;
    right_bound += 30;
    upper_bound -= 30;
    lower_bound += 30;

    // check all the pixels inside the square of these bounds
    uint32_t worker_index = 0;
    for (int32_t y=upper_bound; y <= lower_bound; y+=BUFFER_SLICE) {
        worker_index = (worker_index >= processor_count - 1) ? 0 : worker_index + 1;
        workers[worker_index]->addWorkBlocking({ left_bound, right_bound, y, 
                std::min(y + BUFFER_SLICE, lower_bound), &tr, color, bg_color, buf});
    }

    // wait til all the draws are done
    for (uint32_t i = 0; i < processor_count; i++) {
        while(!workers[i]->isEmpty());
    }
}

void sig_handler(int signo) {
  if (signo == SIGINT) {
      std::cout << " - Received SIGINT, cleaning up." << std::endl;
      keep_running = false;
  }
}

int main(int argc, char **argv) {
	const char *card;

    uint32_t square_dimension = 70;
    uint32_t off_x = 400, off_y = 200;
    uint32_t color_blue = 0x176BEF; // google blue
    uint32_t color_green = 0xFF3E30; // google green
    uint32_t color_yellow = 0xF7B529; // google yellow
    uint32_t color_red = 0x179C52; // google red

    // registar signal handler
    signal(SIGINT, sig_handler);

	/* check which DRM device to open */
	if (argc > 1) {
		card = argv[1];
	} else {
		card = "/dev/dri/card0";
    }

    // init drmUtil on card
    drm_util::DrmUtil drmUtil(card);
    int32_t response;
    if (response = drmUtil.initDrmDev()) {
        return response;
    }

    // draw a triangle and then rotate it
    double smaller_screen_dimension = std::min(drmUtil.mdev->bufs[0].width, drmUtil.mdev->bufs[0].height);
    double trg_offset_x, trg_offset_y, trg_side;
    trg_offset_x = 350;
    trg_offset_y = 150;
    trg_side = 700;
    //trg_offset_x = trg_offset_y = trg_side = 400;
    const double sin60 = sin(60 * M_PI / 180);
    const double cos60 = cos(60 * M_PI / 180);
    double trg_height = trg_side * sin60;
    GM::Triangle trg = GM::Triangle(
        { trg_offset_x + trg_side * cos60,      trg_offset_y,               0 },
        { trg_offset_x,                         trg_offset_y + trg_height,  0 },
        { trg_offset_x + trg_side,              trg_offset_y + trg_height,  0 }
    );
    GM::Triangle new_triangle = GM::Triangle(&trg);

    // start worker threads
    for ( uint32_t i = 0; i < processor_count; i++) {
        workers.push_back(new Szilv::LineDrawer(i));
    }

    int64_t prev_t = get_nanos();
    GM::Vertex center = trg.getCenter();
    int64_t counter = 0;
    drm_util::modeset_buf * buf;
    // rotate triangle
    while (keep_running) {
        int64_t t = get_nanos();
        int64_t t_diff = t - prev_t;
        double angle = (double)(t_diff) * 0.000000001;
        new_triangle.setPrimitive({
            GM::BaseGeometry::rotate(trg.getPrimitive().p1, center, angle),
            GM::BaseGeometry::rotate(trg.getPrimitive().p2, center, angle),
            GM::BaseGeometry::rotate(trg.getPrimitive().p3, center, angle)
        });
        //
        buf = &drmUtil.mdev->bufs[drmUtil.mdev->front_buf ^ 1];
        draw_triangle(buf, new_triangle, trg, color_white);

        // swap buffers
        drmUtil.swap_buffers();


#if(FPS_COUNTER)
        if (counter % 20 == 0) {
            int32_t fps = 1000000000 / t_diff;
            //std::clog << fps << std::endl;
            // TODO: standard IO doesn't work in DRM device, 
            // should create my own FPS counter in the buffer
        }
#endif

        //
        prev_t = t;
        trg = new_triangle;
        counter++;
    }

    // join worker threads
    while (workers.size()) {
        delete workers.back(); // the destructor calls the thread join
        workers.pop_back();
    }

    return 0;
}
