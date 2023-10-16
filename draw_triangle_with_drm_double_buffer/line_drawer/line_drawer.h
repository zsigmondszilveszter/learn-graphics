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
        uint32_t color;
        uint32_t bg_color;
        drm_util::modeset_buf * buf;
        GM::Triangle * tr;
    } TriangleDrawWork;

    typedef struct {
        int32_t left;
        int32_t right;
        int32_t start_line;
        int32_t end_line;
        uint32_t color;
        uint32_t bg_color;
        drm_util::modeset_buf * buf;
        char * digit;
    } FpsDrawWork;


    class LineDrawer {
        public:
            LineDrawer(uint32_t id);
            ~LineDrawer();

            virtual void addTriangleWorkBlocking(TriangleDrawWork work);
            virtual bool addTriangleWorkNonblocking(TriangleDrawWork work);
            virtual uint32_t getTriangleWorkQueueSize();
            virtual bool isTriangleWorkQueueEmpty();

            virtual void addFpsWorkBlocking(FpsDrawWork work);
            virtual bool addFpsWorkNonblocking(FpsDrawWork work);
            virtual uint32_t getFpsWorkQueueSize();
            virtual bool isFpsWorkQueueEmpty();

            virtual void triangleWorker();
            virtual void fpsWorker();

            virtual void executeTriangleDrawWork();
            virtual void executeFpsDigitDrawWork();

        private:
            uint32_t id;
            bool keep_running = true;

            std::binary_semaphore triangle_work_queue_sem{1};
            std::queue<TriangleDrawWork> triangle_work_queue;
            std::thread triangleThd;
            std::binary_semaphore triangle_thread_sem{0};

            std::binary_semaphore fps_work_queue_sem{1};
            std::queue<FpsDrawWork> fps_work_queue;
            std::thread fpsThd;
            std::binary_semaphore fps_thread_sem{0};
    };
}

#endif /* !defined(DRAW_WORKER_H) */
