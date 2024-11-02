/**
 * Szilveszter Zsigmond (zsigmondszilveszter@yahoo.com)
 * Most of the functions are copied over from this example: 
 * https://github.com/dvdhrm/docs/blob/master/drm-howto/modeset.c
 */

#include "drm_util.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cerrno>
#include <iostream>
#include <cstring>

namespace szilv {
    DrmUtil::DrmUtil(const char * card) {
        this->_card = card;
        std::clog << "using card " << card << std::endl;
    }

    /**
     *
     */
    int32_t DrmUtil::modeset_open(int32_t *out, const char *node) {
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

    /**
     *
     */
    void DrmUtil::modeset_cleanup(int32_t fd) {
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

            /* destroy framebuffers */
            modeset_destroy_fb(fd, &iter->bufs[1]);
            modeset_destroy_fb(fd, &iter->bufs[0]);

            /* free allocated memory */
            free(iter);
        }
    }

    /**
     *
     */
    int32_t DrmUtil::modeset_find_crtc(int32_t fd, drmModeRes *res, drmModeConnector *conn, modeset_dev *dev) {
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

    /**
     *
     */
    int32_t DrmUtil::modeset_create_fb(int32_t fd, modeset_buf *buf) {
        struct drm_mode_create_dumb creq;
        struct drm_mode_destroy_dumb dreq;
        struct drm_mode_map_dumb mreq;
        int32_t ret;

        /* create dumb buffer */
        memset(&creq, 0, sizeof(creq));
        creq.width = buf->width;
        creq.height = buf->height;
        creq.bpp = 32;
        ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
        if (ret < 0) {
            std::clog << "cannot create dumb buffer (" << errno << ")" << std::endl;
            return -errno;
        }
        buf->stride = creq.pitch;
        buf->size = creq.size;
        buf->handle = creq.handle;

        /* create framebuffer object for the dumb-buffer */
        ret = drmModeAddFB(fd, buf->width, buf->height, 24, 32, buf->stride,
                buf->handle, &buf->fb);
        if (ret) {
            std::clog << "cannot create framebuffer (" << errno << ")" << std::endl;
            ret = -errno;
            goto err_destroy;
        }

        /* prepare buffer for memory mapping */
        memset(&mreq, 0, sizeof(mreq));
        mreq.handle = buf->handle;
        ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
        if (ret) {
            std::clog << "cannot map dumb buffer (" << errno << ")" << std::endl;
            ret = -errno;
            goto err_fb;
        }

        /* perform actual memory mapping */
        buf->map = (int32_t *) mmap(0, buf->size, PROT_READ | PROT_WRITE, MAP_SHARED,
                fd, mreq.offset);
        if (buf->map == MAP_FAILED) {
            std::clog << "cannot mmap dumb buffer (" << errno << ")" << std::endl;
            ret = -errno;
            goto err_fb;
        }

        /* clear the framebuffer to 0 */
        memset(buf->map, 0, buf->size);

        return 0;

err_fb:
        drmModeRmFB(fd, buf->fb);
err_destroy:
        memset(&dreq, 0, sizeof(dreq));
        dreq.handle = buf->handle;
        drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
        return ret;
    }

    
    /**
     *
     */
    int32_t DrmUtil::modeset_setup_dev(int32_t fd, drmModeRes *res, drmModeConnector *conn,
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
        dev->bufs[0].width = conn->modes[0].hdisplay;
        dev->bufs[0].height = conn->modes[0].vdisplay;
        dev->bufs[1].width = conn->modes[0].hdisplay;
        dev->bufs[1].height = conn->modes[0].vdisplay;
        std::clog << "mode for connector " << conn->connector_id << " is " << dev->bufs[0].width 
            << "*" << dev->bufs[0].height << std::endl;
        std::clog << "connector flag: " << dev->mode.flags << std::endl;

        /* find a crtc for this connector */
        ret = modeset_find_crtc(fd, res, conn, dev);
        if (ret) {
            std::clog << "no valid crtc for connector " << conn->connector_id << std::endl;
            return ret;
        }

        /* create a framebuffer #1 for this CRTC */
        ret = modeset_create_fb(fd, &dev->bufs[0]);
        if (ret) {
            std::clog << "cannot create framebuffer for connector " << conn->connector_id << std::endl;
            return ret;
        }

        /* create a framebuffer #2 for this CRTC */
        ret = modeset_create_fb(fd, &dev->bufs[1]);
        if (ret) {
            std::clog << "cannot create framebuffer for connector " << conn->connector_id << std::endl;
            modeset_destroy_fb(fd, &dev->bufs[0]);
            return ret;
        }

        return 0;
    }

    /**
     *
     */
    void DrmUtil::modeset_destroy_fb(int fd, modeset_buf *buf) {
        struct drm_mode_destroy_dumb dreq;

        /* unmap buffer */
        munmap(buf->map, buf->size);

        /* delete framebuffer */
        drmModeRmFB(fd, buf->fb);

        /* delete dumb buffer */
        memset(&dreq, 0, sizeof(dreq));
        dreq.handle = buf->handle;
        drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
    }

    /**
     *
     */
    int32_t DrmUtil::modeset_prepare(int32_t fd) {
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

    
    void DrmUtil::swap_buffers() {
        modeset_buf * buf = &mdev->bufs[mdev->front_buf ^ 1];
        int32_t ret = drmModeSetCrtc(fd, mdev->crtc, buf->fb, 0, 0,
					     &mdev->conn, 1, &mdev->mode);
        if (ret) {
            std::clog << "cannot flip CRTC for connector " << mdev->conn << " (" << errno << ")" << std::endl;
        } else {
            mdev->front_buf ^= 1;
        }
    }

    /**
     *
     */
    int32_t DrmUtil::initDrmDev() {
        int32_t ret;
        modeset_buf *buf;

        /* open the DRM device */
        ret = modeset_open(&fd, _card);
        if (ret) {
            goto out_return;
        }

        /* prepare all connectors and CRTCs */
        ret = modeset_prepare(fd);
        if (ret) {
            goto out_close;
        }

        /* perform actual modesetting on each found connector+CRTC */
        for (mdev = modeset_list; mdev; mdev = mdev->next) {
            mdev->saved_crtc = drmModeGetCrtc(fd, mdev->crtc);
            buf = &mdev->bufs[mdev->front_buf];
            ret = drmModeSetCrtc(fd, mdev->crtc, buf->fb, 0, 0,
                    &mdev->conn, 1, &mdev->mode);
            if (ret) {
                std::clog << "cannot set CRTC for connector " << mdev->conn << "(" 
                    << errno << ")" << std::endl;
            } else {
                // found an active modeset device
                break;
            }
        }

        return 0;

out_close:
        close(fd);
out_return:
        if (ret) {
            errno = -ret;
            std::clog << "drm_util::modeset failed with error " << errno << std::endl;
        }

        return ret;
    }

    /**
     *
     */
    DrmUtil::~DrmUtil() {
        /* cleanup everything */
        modeset_cleanup(fd);
        close(fd);
        std::clog << "DrmUtil destroyed " << std::endl;
    }
}
