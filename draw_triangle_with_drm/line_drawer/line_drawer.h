#if !defined(DRAW_WORKER_H)
#define DRAW_WORKER_H

#include <queue>
#include <thread>
#include <semaphore>

#include "triangle.h"

namespace Szilv {

    typedef struct {
        int32_t left;
        int32_t right;
        int32_t start_line;
        int32_t end_line;
        GM::Triangle * tr;
        uint32_t color;
        uint32_t bg_color;
    } TriangleDrawWorkFrameBuffer;


    class LineDrawer {
        public:
            LineDrawer(uint32_t id, int32_t w, int32_t h, uint32_t * framebuffer);
            ~LineDrawer();

            virtual void addWorkBlocking(TriangleDrawWorkFrameBuffer work);
            virtual bool addWorkNonblocking(TriangleDrawWorkFrameBuffer work);

            virtual uint32_t getWorkQueueSize();
            virtual bool isEmpty();

            virtual void worker();

            virtual void executeTriangleDrawWorkInFrameBuffer();

        private:
            uint32_t id;
            bool keep_running = true;

            // framebuffer size
            int32_t fb_width;
            int32_t fb_height;
            // framebuffer data pointer
            uint32_t * fbdata;

            std::binary_semaphore thread_sem{0};

            std::binary_semaphore triangle_work_queue_sem{1};
            std::queue<TriangleDrawWorkFrameBuffer> triangle_work_queue;

            std::thread thd;
    };
}

#endif /* !defined(DRAW_WORKER_H) */
