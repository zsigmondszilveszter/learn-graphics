#if !defined(DRAW_WORKER_H)
#define DRAW_WORKER_H

#include <queue>
#include <thread>
#include <semaphore>

#include "drm_util.h"

namespace SG {

    enum WorkType {
        Triangle,
        Digit
    };

    typedef struct {
        int32_t left;
        int32_t right;
        int32_t start_line;
        int32_t end_line;
        uint32_t color;
        uint32_t bg_color;
        drm_util::modeset_buf * buf;
        WorkType workType;
        void * obj;
    } DrawWork;

    class LineDrawer {
        public:
            LineDrawer(uint32_t id);
            ~LineDrawer();

            virtual void addWorkBlocking(DrawWork work);
            virtual bool addWorkNonblocking(DrawWork work);
            virtual uint32_t getWorkQueueSize();
            virtual bool isWorkQueueEmpty();

            virtual void threadWorker();
            virtual void blockMainThreadUntilTheQueueIsNotEmpty();

        private:
            uint32_t id;
            bool keep_running = true;

            std::binary_semaphore sem_work_queue{1};
            std::queue<DrawWork> work_queue;
            std::thread thd;
            std::binary_semaphore sem_block_this_thread{0};
            std::binary_semaphore sem_block_main_thread{1};
    };
}

#endif /* !defined(DRAW_WORKER_H) */
