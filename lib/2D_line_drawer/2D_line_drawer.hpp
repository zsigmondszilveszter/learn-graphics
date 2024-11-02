#if !defined(DRAW_WORKER_H)
#define DRAW_WORKER_H

#include <queue>
#include <thread>
#include <semaphore.h>
#include <functional>
#include "base_geometry.hpp"


namespace szilv {

    typedef struct {
        int32_t x1, y1;
        int32_t x2, y2;
    } SquareDefinition;

    typedef struct {
        uint32_t color;
        uint32_t bg_color;
        void * obj;
        std::function<bool(szilv::Vertex)> isInside;
        SquareDefinition squareDefinition;
        int32_t * target_buff = nullptr;
        uint32_t buff_width;
        uint32_t buff_height;
    } DrawWork;

    class LineDrawer2D {
        public:
            LineDrawer2D(uint32_t id, uint32_t x, uint32_t y);
            ~LineDrawer2D();

            virtual void addWorkBlocking(DrawWork work);
            virtual uint32_t getWorkQueueSize();
            virtual bool isWorkQueueEmpty();

            virtual void threadWorker();
            virtual void blockMainThreadUntilTheQueueIsNotEmpty();

        private:
            uint32_t id;
            bool keep_running = true;

            std::queue<DrawWork> work_queue;
            std::thread thd;
            sem_t sem_work_queue{1};
            sem_t sem_block_this_thread{0};
            sem_t sem_block_main_thread{1};
    };
}

#endif /* !defined(DRAW_WORKER_H) */
