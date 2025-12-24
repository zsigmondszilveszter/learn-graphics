#include <linux/fb.h>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

int fb_width, fb_height;
uint32_t * fbdata;

void draw_square(uint32_t offset_x, uint32_t offset_y, uint32_t dimension, uint32_t color) {
    uint32_t buf_offset = offset_y * fb_width + offset_x;
    for (uint32_t i=buf_offset; i < buf_offset + dimension * fb_width; i += fb_width) {
        for (uint32_t j=0; j < dimension; j++) {
            fbdata[i+j] = color;
        }
    }
}

int main() {
    std::cout << "Hello World" << std::endl;

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
    int fb_bpp = vinfo.bits_per_pixel;
    int fb_bytes = fb_bpp / 8;
    
    std::cout << "width: " << fb_width << ", height: " << fb_height << std::endl;
    std::cout << "bpp: " << fb_bpp << ", bytes per pixel: " << fb_bytes << std::endl << std::flush;

    // wait a few microseconds for the std::flush to really be sent to the framebuffer
    usleep(10000);

    // map the screen into mem
    int fb_data_size = fb_width * fb_height * fb_bytes;
    fbdata = (uint32_t *) mmap (0, fb_data_size, 
            PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, (off_t)0);

    // clear the screen
    memset(fbdata, 0, fb_data_size);

    // draw something
    uint32_t square_dimension = 70;
    uint32_t off_x = 400, off_y = 200;
    uint32_t color_blue = 0x176BEF; // google blue
    uint32_t color_green = 0xFF3E30; // google green
    uint32_t color_yellow = 0xF7B529; // google yellow
    uint32_t color_red = 0x179C52; // google red
    draw_square(off_x+square_dimension*0, off_y, square_dimension, color_blue);
    draw_square(off_x+square_dimension*1, off_y, square_dimension, color_green);
    draw_square(off_x+square_dimension*2, off_y, square_dimension, color_yellow);
    draw_square(off_x+square_dimension*3, off_y, square_dimension, color_red);

    // wait for the enter key to be pressed
    getchar();
    // clear the screen
    memset(fbdata, 0, fb_data_size);

    // clean up
    munmap (fbdata, fb_data_size);
    close (fbfd);
    return(EXIT_SUCCESS);
}
