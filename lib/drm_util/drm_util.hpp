#if !defined(DRM_UTIL_H)
#define DRM_UTIL_H

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <cstdint>

namespace szilv {

    typedef struct modeset_buf modeset_buf;
    struct modeset_buf {
        uint32_t width;
        uint32_t height;
        uint32_t stride;
        uint32_t size;
        uint32_t handle;
        int32_t *map;
        uint32_t fb;
    };

    typedef struct modeset_dev modeset_dev;
    struct modeset_dev {
        modeset_dev *next;
        
        int32_t front_buf;
        modeset_buf bufs[2];

        drmModeModeInfo mode;
        uint32_t conn;
        uint32_t crtc;
        drmModeCrtc *saved_crtc;
    };

    class DrmUtil {
        public:
            DrmUtil(const char * card);
            ~DrmUtil();
            modeset_dev * mdev;
            virtual int32_t initDrmDev();
            virtual void swap_buffers();

        private:
            const char * _card;
            int32_t fd;
            modeset_dev *modeset_list = NULL;

            virtual int32_t modeset_open(int32_t *out, const char *node);
            virtual void modeset_cleanup(int32_t fd);
            virtual int32_t modeset_find_crtc(int32_t fd, drmModeRes *res, 
                    drmModeConnector *conn, modeset_dev *dev);
            virtual int32_t modeset_create_fb(int32_t fd, modeset_buf *dev);
            virtual int32_t modeset_setup_dev(int32_t fd, drmModeRes *res, drmModeConnector *conn, 
                    modeset_dev *dev);
            virtual void modeset_destroy_fb(int fd, modeset_buf *buf);
            virtual int32_t modeset_prepare(int32_t fd);
    };
}

#endif /* !defined(DRM_UTIL_H) */
