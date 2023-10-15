/**
 * Szilveszter Zsigmond (zsigmondszilveszter@yahoo.com)
 * Most of the explanations are copied over from this example: 
 * https://github.com/dvdhrm/docs/blob/master/drm-howto/modeset.c
 */

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <iostream>
#include <cstdint>
#include <cerrno>
#include <cstring>

/*
 * When the linux kernel detects a graphics-card on your machine, it loads the
 * correct device driver (located in kernel-tree at ./drivers/gpu/drm/<xy>) and
 * provides two character-devices to control it. Udev (or whatever hotplugging
 * application you use) will create them as:
 *     /dev/dri/card0
 *     /dev/dri/controlID64
 * We only need the first one. You can hard-code this path into your application
 * like we do here, but it is recommended to use libudev with real hotplugging
 * and multi-seat support. However, this is beyond the scope of this document.
 * Also note that if you have multiple graphics-cards, there may also be
 * /dev/dri/card1, /dev/dri/card2, ...
 *
 * We simply use /dev/dri/card0 here but the user can specify another path on
 * the command line.
 *
 * modeset_open(out, node): This small helper function opens the DRM device
 * which is given as @node. The new fd is stored in @out on success. On failure,
 * a negative error code is returned.
 * After opening the file, we also check for the DRM_CAP_DUMB_BUFFER capability.
 * If the driver supports this capability, we can create simple memory-mapped
 * buffers without any driver-dependent code. As we want to avoid any radeon,
 * nvidia, intel, etc. specific code, we depend on DUMB_BUFFERs here.
 */

static int32_t modeset_open(int32_t *out, const char *node) {
	int32_t fd, ret;
	uint64_t has_dumb;

	fd = open(node, O_RDWR | O_CLOEXEC);
	if (fd < 0) {
		ret = -errno;
        std::clog << "cannot open '" << node << "': " << errno << std::endl;
		return ret;
	}

	if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 ||
	    !has_dumb) {
        std::clog << "drm device '" << node << "' does not support dumb buffers" << std::endl;
		close(fd);
		return -EOPNOTSUPP;
	}

	*out = fd;
	return 0;
}

/*
 * As a next step we need to find our available display devices. libdrm provides
 * a drmModeRes structure that contains all the needed information. We can
 * retrieve it via drmModeGetResources(fd) and free it via
 * drmModeFreeResources(res) again.
 *
 * A physical connector on your graphics card is called a "connector". You can
 * plug a monitor into it and control what is displayed. We are definitely
 * interested in what connectors are currently used, so we simply iterate
 * through the list of connectors and try to display a test-picture on each
 * available monitor.
 * However, this isn't as easy as it sounds. First, we need to check whether the
 * connector is actually used (a monitor is plugged in and turned on). Then we
 * need to find a CRTC that can control this connector. CRTCs are described
 * later on. After that we create a framebuffer object. If we have all this, we
 * can mmap() the framebuffer and draw a test-picture into it. Then we can tell
 * the DRM device to show the framebuffer on the given CRTC with the selected
 * connector.
 *
 * As we want to draw moving pictures on the framebuffer, we actually have to
 * remember all these settings. Therefore, we create one "struct modeset_dev"
 * object for each connector+crtc+framebuffer pair that we successfully
 * initialized and push it into the global device-list.
 *
 * Each field of this structure is described when it is first used. But as a
 * summary:
 * "struct modeset_dev" contains: {
 *  - @next: points to the next device in the single-linked list
 *
 *  - @width: width of our buffer object
 *  - @height: height of our buffer object
 *  - @stride: stride value of our buffer object
 *  - @size: size of the memory mapped buffer
 *  - @handle: a DRM handle to the buffer object that we can draw into
 *  - @map: pointer to the memory mapped buffer
 *
 *  - @mode: the display mode that we want to use
 *  - @fb: a framebuffer handle with our buffer object as scanout buffer
 *  - @conn: the connector ID that we want to use with this buffer
 *  - @crtc: the crtc ID that we want to use with this connector
 *  - @saved_crtc: the configuration of the crtc before we changed it. We use it
 *                 so we can restore the same mode when we exit.
 * }
 */
typedef struct modeset_dev modeset_dev;

struct modeset_dev {
	modeset_dev *next;

	uint32_t width;
	uint32_t height;
	uint32_t stride;
	uint32_t size;
	uint32_t handle;
	uint32_t *map;

	drmModeModeInfo mode;
	uint32_t fb;
	uint32_t conn;
	uint32_t crtc;
	drmModeCrtc *saved_crtc;
};

static modeset_dev *modeset_list = NULL;

/*
 * modeset_find_crtc(fd, res, conn, dev): This small helper tries to find a
 * suitable CRTC for the given connector. We have actually have to introduce one
 * more DRM object to make this more clear: Encoders.
 * Encoders help the CRTC to convert data from a framebuffer into the right
 * format that can be used for the chosen connector. We do not have to
 * understand any more of these conversions to make use of it. However, you must
 * know that each connector has a limited list of encoders that it can use. And
 * each encoder can only work with a limited list of CRTCs. So what we do is
 * trying each encoder that is available and looking for a CRTC that this
 * encoder can work with. If we find the first working combination, we are happy
 * and write it into the @dev structure.
 * But before iterating all available encoders, we first try the currently
 * active encoder+crtc on a connector to avoid a full modeset.
 *
 * However, before we can use a CRTC we must make sure that no other device,
 * that we setup previously, is already using this CRTC. Remember, we can only
 * drive one connector per CRTC! So we simply iterate through the "modeset_list"
 * of previously setup devices and check that this CRTC wasn't used before.
 * Otherwise, we continue with the next CRTC/Encoder combination.
 */

static int32_t modeset_find_crtc(int32_t fd, drmModeRes *res, drmModeConnector *conn,
			     modeset_dev *dev) {
	drmModeEncoder *enc;
	uint32_t i, j;
	int32_t crtc;
	modeset_dev *iter;

	/* first try the currently conected encoder+crtc */
	if (conn->encoder_id) {
		enc = drmModeGetEncoder(fd, conn->encoder_id);
    } else {
		enc = NULL;
    }

	if (enc) {
		if (enc->crtc_id) {
			crtc = enc->crtc_id;
			for (iter = modeset_list; iter; iter = iter->next) {
				if (iter->crtc == crtc) {
					crtc = -1;
					break;
				}
			}

			if (crtc >= 0) {
				drmModeFreeEncoder(enc);
				dev->crtc = crtc;
				return 0;
			}
		}

		drmModeFreeEncoder(enc);
	}

	/* If the connector is not currently bound to an encoder or if the
	 * encoder+crtc is already used by another connector (actually unlikely
	 * but lets be safe), iterate all other available encoders to find a
	 * matching CRTC. */
	for (i = 0; i < conn->count_encoders; ++i) {
		enc = drmModeGetEncoder(fd, conn->encoders[i]);
		if (!enc) {
            std::clog << "cannot retrieve encoder " << i << ":" << conn->encoders[i] 
                << " (" << errno << ")" << std::endl;
			continue;
		}

		/* iterate all global CRTCs */
		for (j = 0; j < res->count_crtcs; ++j) {
			/* check whether this CRTC works with the encoder */
			if (!(enc->possible_crtcs & (1 << j)))
				continue;

			/* check that no other device already uses this CRTC */
			crtc = res->crtcs[j];
			for (iter = modeset_list; iter; iter = iter->next) {
				if (iter->crtc == crtc) {
					crtc = -1;
					break;
				}
			}

			/* we have found a CRTC, so save it and return */
			if (crtc >= 0) {
				drmModeFreeEncoder(enc);
				dev->crtc = crtc;
				return 0;
			}
		}

		drmModeFreeEncoder(enc);
	}

    std::clog << "cannot find suitable CRTC for connector " << conn->connector_id << std::endl;
	return -ENOENT;
}

/*
 * modeset_create_fb(fd, dev): After we have found a crtc+connector+mode
 * combination, we need to actually create a suitable framebuffer that we can
 * use with it. There are actually two ways to do that:
 *   * We can create a so called "dumb buffer". This is a buffer that we can
 *     memory-map via mmap() and every driver supports this. We can use it for
 *     unaccelerated software rendering on the CPU.
 *   * We can use libgbm to create buffers available for hardware-acceleration.
 *     libgbm is an abstraction layer that creates these buffers for each
 *     available DRM driver. As there is no generic API for this, each driver
 *     provides its own way to create these buffers.
 *     We can then use such buffers to create OpenGL contexts with the mesa3D
 *     library.
 * We use the first solution here as it is much simpler and doesn't require any
 * external libraries. However, if you want to use hardware-acceleration via
 * OpenGL, it is actually pretty easy to create such buffers with libgbm and
 * libEGL. But this is beyond the scope of this document.
 *
 * So what we do is requesting a new dumb-buffer from the driver. We specify the
 * same size as the current mode that we selected for the connector.
 * Then we request the driver to prepare this buffer for memory mapping. After
 * that we perform the actual mmap() call. So we can now access the framebuffer
 * memory directly via the dev->map memory map.
 */

static int32_t modeset_create_fb(int32_t fd, modeset_dev *dev) {
	struct drm_mode_create_dumb creq;
	struct drm_mode_destroy_dumb dreq;
	struct drm_mode_map_dumb mreq;
	int32_t ret;

	/* create dumb buffer */
	memset(&creq, 0, sizeof(creq));
	creq.width = dev->width;
	creq.height = dev->height;
	creq.bpp = 32;
	ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
	if (ret < 0) {
        std::clog << "cannot create dumb buffer (" << errno << ")" << std::endl;
		return -errno;
	}
	dev->stride = creq.pitch;
	dev->size = creq.size;
	dev->handle = creq.handle;

	/* create framebuffer object for the dumb-buffer */
	ret = drmModeAddFB(fd, dev->width, dev->height, 24, 32, dev->stride,
			   dev->handle, &dev->fb);
	if (ret) {
        std::clog << "cannot create framebuffer (" << errno << ")" << std::endl;
		ret = -errno;
		goto err_destroy;
	}

	/* prepare buffer for memory mapping */
	memset(&mreq, 0, sizeof(mreq));
	mreq.handle = dev->handle;
	ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
	if (ret) {
        std::clog << "cannot map dumb buffer (" << errno << ")" << std::endl;
		ret = -errno;
		goto err_fb;
	}

	/* perform actual memory mapping */
	dev->map = (uint32_t *) mmap(0, dev->size, PROT_READ | PROT_WRITE, MAP_SHARED,
		        fd, mreq.offset);
	if (dev->map == MAP_FAILED) {
        std::clog << "cannot mmap dumb buffer (" << errno << ")" << std::endl;
		ret = -errno;
		goto err_fb;
	}

	/* clear the framebuffer to 0 */
	memset(dev->map, 0, dev->size);

	return 0;

err_fb:
	drmModeRmFB(fd, dev->fb);
err_destroy:
	memset(&dreq, 0, sizeof(dreq));
	dreq.handle = dev->handle;
	drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
	return ret;
}

/*
 * Now we dig deeper into setting up a single connector. As described earlier,
 * we need to check several things first:
 *   * If the connector is currently unused, that is, no monitor is plugged in,
 *     then we can ignore it.
 *   * We have to find a suitable resolution and refresh-rate. All this is
 *     available in drmModeModeInfo structures saved for each crtc. We simply
 *     use the first mode that is available. This is always the mode with the
 *     highest resolution.
 *     A more sophisticated mode-selection should be done in real applications,
 *     though.
 *   * Then we need to find an CRTC that can drive this connector. A CRTC is an
 *     internal resource of each graphics-card. The number of CRTCs controls how
 *     many connectors can be controlled indepedently. That is, a graphics-cards
 *     may have more connectors than CRTCs, which means, not all monitors can be
 *     controlled independently.
 *     There is actually the possibility to control multiple connectors via a
 *     single CRTC if the monitors should display the same content. However, we
 *     do not make use of this here.
 *     So think of connectors as pipelines to the connected monitors and the
 *     CRTCs are the controllers that manage which data goes to which pipeline.
 *     If there are more pipelines than CRTCs, then we cannot control all of
 *     them at the same time.
 *   * We need to create a framebuffer for this connector. A framebuffer is a
 *     memory buffer that we can write XRGB32 data into. So we use this to
 *     render our graphics and then the CRTC can scan-out this data from the
 *     framebuffer onto the monitor.
 */

static int32_t modeset_setup_dev(int32_t fd, drmModeRes *res, drmModeConnector *conn,
			     modeset_dev *dev) {
	int32_t ret;

	/* check if a monitor is connected */
	if (conn->connection != DRM_MODE_CONNECTED) {
        std::clog << "ignoring unused connector " << conn->connector_id << std::endl;
		return -ENOENT;
	}

	/* check if there is at least one valid mode */
	if (conn->count_modes == 0) {
        std::clog << "no valid mode for connector " << conn->connector_id << std::endl;
		return -EFAULT;
	}

	/* copy the mode information into our device structure */
	memcpy(&dev->mode, &conn->modes[0], sizeof(dev->mode));
	dev->width = conn->modes[0].hdisplay;
	dev->height = conn->modes[0].vdisplay;
    std::clog << "mode for connector " << conn->connector_id << " is " << dev->width 
        << "*" << dev->height << std::endl;

	/* find a crtc for this connector */
	ret = modeset_find_crtc(fd, res, conn, dev);
	if (ret) {
        std::clog << "no valid crtc for connector " << conn->connector_id << std::endl;
		return ret;
	}

	/* create a framebuffer for this CRTC */
	ret = modeset_create_fb(fd, dev);
	if (ret) {
        std::clog << "cannot create framebuffer for connector " << conn->connector_id << std::endl;
		return ret;
	}

	return 0;
}

/*
 * So as next step we need to actually prepare all connectors that we find. We
 * do this in this little helper function:
 *
 * modeset_prepare(fd): This helper function takes the DRM fd as argument and
 * then simply retrieves the resource-info from the device. It then iterates
 * through all connectors and calls other helper functions to initialize this
 * connector (described later on).
 * If the initialization was successful, we simply add this object as new device
 * into the global modeset device list.
 *
 * The resource-structure contains a list of all connector-IDs. We use the
 * helper function drmModeGetConnector() to retrieve more information on each
 * connector. After we are done with it, we free it again with
 * drmModeFreeConnector().
 * Our helper modeset_setup_dev() returns -ENOENT if the connector is currently
 * unused and no monitor is plugged in. So we can ignore this connector.
 */

static int32_t modeset_prepare(int32_t fd) {
	drmModeRes *res;
	drmModeConnector *conn;
	uint32_t i;
	modeset_dev *dev;
	int32_t ret;

	/* retrieve resources */
	res = drmModeGetResources(fd);
	if (!res) {
        std::clog << "cannot retrieve DRM resources: " << errno << std::endl;
		return -errno;
	}

	/* iterate all connectors */
	for (i = 0; i < res->count_connectors; ++i) {
		/* get information for each connector */
		conn = drmModeGetConnector(fd, res->connectors[i]);
		if (!conn) {
            std::clog << "cannot retrieve DRM connector " << i << ":" 
                << res->connectors[i] << " (" << errno << ")" << std::endl;
			continue;
		}

		/* create a device structure */
		dev = (modeset_dev *) malloc(sizeof(*dev));
		memset(dev, 0, sizeof(*dev));
		dev->conn = conn->connector_id;

		/* call helper function to prepare this connector */
		ret = modeset_setup_dev(fd, res, conn, dev);
		if (ret) {
			if (ret != -ENOENT) {
				errno = -ret;
                std::clog << "cannot setup device for connector " << i << ":" 
                    << res->connectors[i] << "(" << errno << ")" << std::endl;
			}
			free(dev);
			drmModeFreeConnector(conn);
			continue;
		}

		/* free connector data and link device into global list */
		drmModeFreeConnector(conn);
		dev->next = modeset_list;
		modeset_list = dev;
	}

	/* free resources again */
	drmModeFreeResources(res);
	return 0;
}

/*
 * modeset_cleanup(fd): This cleans up all the devices we created during
 * modeset_prepare(). It resets the CRTCs to their saved states and deallocates
 * all memory.
 * It should be pretty obvious how all of this works.
 */

static void modeset_cleanup(int32_t fd) {
	modeset_dev *iter;
	struct drm_mode_destroy_dumb dreq;

	while (modeset_list) {
		/* remove from global list */
		iter = modeset_list;
		modeset_list = iter->next;

		/* restore saved CRTC configuration */
		drmModeSetCrtc(fd,
			       iter->saved_crtc->crtc_id,
			       iter->saved_crtc->buffer_id,
			       iter->saved_crtc->x,
			       iter->saved_crtc->y,
			       &iter->conn,
			       1,
			       &iter->saved_crtc->mode);
		drmModeFreeCrtc(iter->saved_crtc);

		/* unmap buffer */
		munmap(iter->map, iter->size);

		/* delete framebuffer */
		drmModeRmFB(fd, iter->fb);

		/* delete dumb buffer */
		memset(&dreq, 0, sizeof(dreq));
		dreq.handle = iter->handle;
		drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);

		/* free allocated memory */
		free(iter);
	}
}


void draw_square(modeset_dev *mdev, uint32_t offset_x, uint32_t offset_y, 
        uint32_t dimension, uint32_t color) {
    uint32_t buf_offset = offset_y * mdev->width + offset_x;
    for (uint32_t i=buf_offset; i < buf_offset + dimension * mdev->width; i += mdev->width) {
        for (uint32_t j=0; j < dimension; j++) {
            mdev->map[i+j] = color;
        }
    }
}

int main(int argc, char **argv) {
    int32_t ret, fd;
	const char *card;
	modeset_dev * iter;
    modeset_dev * dev;

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

    std::clog << "using card " << card << std::endl;

    /* open the DRM device */
	ret = modeset_open(&fd, card);
	if (ret) {
		goto out_return;
    }

	/* prepare all connectors and CRTCs */
	ret = modeset_prepare(fd);
	if (ret) {
		goto out_close;
    }

	/* perform actual modesetting on each found connector+CRTC */
	for (iter = modeset_list; iter; iter = iter->next) {
		iter->saved_crtc = drmModeGetCrtc(fd, iter->crtc);
		ret = drmModeSetCrtc(fd, iter->crtc, iter->fb, 0, 0,
				     &iter->conn, 1, &iter->mode);
		if (ret) {
            std::clog << "cannot set CRTC for connector " << iter->conn << "(" 
                << errno << ")" << std::endl;
        } else {
            dev = iter;
        }
	}

    // draw something
    draw_square(dev, off_x+square_dimension*0, off_y, square_dimension, color_blue);
    draw_square(dev, off_x+square_dimension*1, off_y, square_dimension, color_green);
    draw_square(dev, off_x+square_dimension*2, off_y, square_dimension, color_yellow);
    draw_square(dev, off_x+square_dimension*3, off_y, square_dimension, color_red);

    // wait for the enter key to be pressed
    getchar();

    /* cleanup everything */
	modeset_cleanup(fd);

	ret = 0;

out_close:
	close(fd);
out_return:
	if (ret) {
		errno = -ret;
        std::clog << "modeset failed with error " << errno << std::endl;
	} else {
        std::clog << "exiting" << std::endl;
	}

    return ret;
}
