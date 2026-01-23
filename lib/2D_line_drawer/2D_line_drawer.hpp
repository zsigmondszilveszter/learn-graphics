#if !defined(DRAW_WORKER_H)
#define DRAW_WORKER_H

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include "base_geometry.hpp"


namespace szilv {

    struct DrawWorkStruct {
        uint32_t color;
        uint32_t bg_color;
        void * obj;
        std::function<bool(szilv::Vertex)> isInside;
        SquareDefinition squareDefinition;
        uint8_t * target_buff = nullptr;
        uint32_t pitch;
        uint32_t buff_width;
        uint32_t buff_height;
    };
    typedef DrawWorkStruct DrawWork;

    class Semaphore {
    public:
        Semaphore(int count_ = 0) : count(count_) {}

        void notify() {
            std::unique_lock<std::mutex> lock(mtx);
            count++;
            cv.notify_one();
        }

        void wait() {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this]() { return count > 0; });
            count--;
        }

    private:
        std::mutex mtx;
        std::condition_variable cv;
        int count;
    };

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
            Semaphore sem_work_queue;
            Semaphore sem_block_this_thread;
            Semaphore sem_block_main_thread;
    };
}

#endif /* !defined(DRAW_WORKER_H) */
