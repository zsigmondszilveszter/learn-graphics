#include <unistd.h>
#include <iostream>

#include "drm_util.h"

void draw_square(drm_util::modeset_dev *mdev, uint32_t offset_x, uint32_t offset_y, 
        uint32_t dimension, uint32_t color) {
    uint32_t buf_offset = offset_y * mdev->width + offset_x;
    for (uint32_t i=buf_offset; i < buf_offset + dimension * mdev->width; i += mdev->width) {
        for (uint32_t j=0; j < dimension; j++) {
            mdev->map[i+j] = color;
        }
    }
}

int main(int argc, char **argv) {
	const char *card;
    drm_util::modeset_dev * dev;

    uint32_t square_dimension = 70;
    uint32_t off_x = 400, off_y = 200;
    uint32_t color_blue = 0x176BEF; // google blue
    uint32_t color_green = 0xFF3E30; // google green
    uint32_t color_yellow = 0xF7B529; // google yellow
    uint32_t color_red = 0x179C52; // google red
                                   //
	/* check which DRM device to open */
	if (argc > 1) {
		card = argv[1];
	} else {
		card = "/dev/dri/card0";
    }

    drm_util::DrmUtil drmUtil(card);

    int32_t response;
    if (response = drmUtil.initDrmDev()) {
        return response;
    }

    // draw something
    draw_square(drmUtil.mdev, off_x+square_dimension*0, off_y, square_dimension, color_blue);
    draw_square(drmUtil.mdev, off_x+square_dimension*1, off_y, square_dimension, color_green);
    draw_square(drmUtil.mdev, off_x+square_dimension*2, off_y, square_dimension, color_yellow);
    draw_square(drmUtil.mdev, off_x+square_dimension*3, off_y, square_dimension, color_red);

    // wait for the enter key to be pressed
    getchar();

    return 0;
}
