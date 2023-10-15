#if !defined(DRAW_WORKER_H)
#define DRAW_WORKER_H

#include <queue>
#include <thread>
#include <semaphore>

#include "triangle.h"
#include "drm_util.h"

namespace Szilv {

    typedef struct {
        int32_t left;
        int32_t right;
        int32_t start_line;
        int32_t end_line;
        GM::Triangle * tr;
        uint32_t color;
        uint32_t bg_color;
        drm_util::modeset_buf * buf;
    } TriangleDrawWork;


    class LineDrawer {
        public:
            LineDrawer(uint32_t id);
            ~LineDrawer();

            virtual void addWorkBlocking(TriangleDrawWork work);
            virtual bool addWorkNonblocking(TriangleDrawWork work);

            virtual uint32_t getWorkQueueSize();
            virtual bool isEmpty();

            virtual void worker();

            virtual void executeTriangleDrawWork();

        private:
            uint32_t id;
            bool keep_running = true;

            std::binary_semaphore thread_sem{0};

            std::binary_semaphore triangle_work_queue_sem{1};
            std::queue<TriangleDrawWork> triangle_work_queue;

            std::thread thd;
    };
}

#endif /* !defined(DRAW_WORKER_H) */
