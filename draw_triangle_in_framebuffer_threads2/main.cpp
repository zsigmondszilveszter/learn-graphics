#include <unistd.h>
#include <iostream>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <execinfo.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <cstring>

#include "line_drawer.h"
#include "base_geometry.h"
#include "fps_digits.h"
#include "triangle.h"

#define BUFFER_SLICE 10
#define BUFFER_UPPER_LIMIT 1000
#define FPS_COUNTER true


uint32_t color_blue     = 0x4285f4; // google blue
uint32_t color_green    = 0x0F9D58; // google green
uint32_t color_yellow   = 0xF4B400; // google yellow
uint32_t color_red      = 0xDB4437; // google red
uint32_t color_white    = 0xFFFFFF;
uint32_t color_black    = 0x0;
uint32_t fb_width, fb_height;
uint32_t * fbdata;
bool keep_running = true;
const uint32_t nr_of_draw_workers = std::min(4U, std::thread::hardware_concurrency() - 1);
std::vector<SG::LineDrawer *> workers;

static int64_t get_nanos(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (int64_t)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

void draw_triangle(uint32_t * buf, GM::Triangle tr, GM::Triangle old_tr, uint32_t color) {
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

    // increase the box a little bit to surely cover the previous triangle
    left_bound  -= 60;
    right_bound += 60;
    upper_bound -= 60;
    lower_bound += 60;

    // check all the pixels inside the square of these bounds
    uint32_t slice = 0;
    for (int32_t y=upper_bound; y <= lower_bound; y+=BUFFER_SLICE) {
        workers[slice % nr_of_draw_workers]->addWorkBlocking({ left_bound, right_bound, y, 
                std::min(y + BUFFER_SLICE, lower_bound), color, bg_color, buf, fb_width, fb_height, 
                SG::Triangle, (void*)&tr});
        slice++;
    }
}

#if(FPS_COUNTER)
uint32_t fps = 0;
uint32_t max_nr_of_digits = 0;
uint64_t counter = 2;
void fps_counter(uint32_t * buf, int64_t t_diff) {
    if (counter % 10 == 0) {
        fps = 1000000000 / t_diff;
    }
    uint32_t nr_of_digits = 0;
    uint32_t tmp = fps;
    while (tmp) {
        char * digit = GM::FpsDigits::getDigit(tmp % 10);
        int32_t left = 1920 - 15 * ((int32_t)nr_of_digits + 1) - 3 * (int32_t)nr_of_digits;
        workers[nr_of_digits % nr_of_draw_workers]->addWorkBlocking(
                { left, left + 15, 2, 20, color_blue, color_black, buf, 
                fb_width, fb_height, SG::Digit, (void*)digit});
        tmp /= 10; 
        nr_of_digits++;
    }
    max_nr_of_digits = std::max(max_nr_of_digits, nr_of_digits);
    while (nr_of_digits < max_nr_of_digits) {
        char * digit = (char*)GM::FpsDigits::blank;
        int32_t left = 1920 - 15 * ((int32_t)nr_of_digits + 1) - 3 * (int32_t)nr_of_digits;
        workers[nr_of_digits % nr_of_draw_workers]->addWorkBlocking(
                { left, left + 15, 2, 20, color_blue, color_black, buf, 
                fb_width, fb_height, SG::Digit, (void*)digit});
        nr_of_digits++;
    }

    counter++;
}
#endif

void sig_handler(int signo) {
    if (signo == SIGINT) {
        std::cerr << " - Received SIGINT, cleaning up." << std::endl;
        keep_running = false;

        void *array[10];
        size_t size;

        // get void*'s for all entries on the stack
        size = backtrace(array, 10);

        // print out all the frames to stderr
        std::cerr << "Error: signal " << signo << std::endl;
        backtrace_symbols_fd(array, size, STDERR_FILENO);
        exit(1);
    }
}

int main(int argc, char **argv) {
    // registar signal handler
    signal(SIGINT, sig_handler);

    // open framebuffer device read/write mode
    int fbfd = open ("/dev/fb0", O_RDWR);
    if (fbfd < 0) {
        std::clog << "Couldn't open the framebuffer device, errno: " << errno << std::endl;
        return(EXIT_FAILURE);
    }
    
    // find the dimentions of the screen
    struct fb_var_screeninfo vinfo;
    ioctl (fbfd, FBIOGET_VSCREENINFO, &vinfo);
    fb_width = vinfo.xres;
    fb_height = vinfo.yres;
    int32_t fb_bpp = vinfo.bits_per_pixel;
    int32_t fb_bytes = fb_bpp / 8;
    
    // map the screen into mem
    int32_t fb_data_size = fb_width * fb_height * fb_bytes;
    fbdata = (uint32_t *) mmap (0, fb_data_size, 
            PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, (off_t)0);

    // clear the screen
    memset(fbdata, 0, fb_data_size);

    // draw a triangle and then rotate it
    double smaller_screen_dimension = std::min(fb_width, fb_height);
    double trg_offset_x, trg_offset_y, trg_side;
    //trg_offset_x = 350;
    //trg_offset_y = 150;
    //trg_side = 700;
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

    // start worker threads
    for ( uint32_t i = 0; i < nr_of_draw_workers; i++) {
        workers.push_back(new SG::LineDrawer(i));
    }

    int64_t prev_t = get_nanos();
    GM::Vertex center = trg.getCenter();
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
        draw_triangle(fbdata, new_triangle, trg, color_white);

#if(FPS_COUNTER)
        fps_counter(fbdata, t_diff);
#endif

        // wait til all the draws are done
        for (uint32_t i = 0; i < nr_of_draw_workers; i++) {
            workers[i]->blockMainThreadUntilTheQueueIsNotEmpty();
        }

        //
        prev_t = t;
        trg = new_triangle;
    }

    // join worker threads
    while (workers.size()) {
        delete workers.back(); // the destructor calls the thread join
        workers.pop_back();
    }

    // clean up
    munmap (fbdata, fb_data_size);
    close (fbfd);
    return(EXIT_SUCCESS);
}
