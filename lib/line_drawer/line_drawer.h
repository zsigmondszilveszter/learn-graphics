#if !defined(DRAW_WORKER_H)
#define DRAW_WORKER_H

#include <queue>
#include <thread>
#include <semaphore.h>


namespace SG {

    enum WorkType {
        Triangle,
        Digit
    };

    typedef struct {
        int32_t x1, y1;
        int32_t x2, y2;
    } SquareDefinition;

    typedef struct {
        uint32_t width;
        uint32_t height;
        int32_t * map;
    } DrawBuffer;

    typedef struct {
        SquareDefinition squareDefinition;
        uint32_t color;
        uint32_t bg_color;
        WorkType workType;
        void * obj;
        int32_t * buff = nullptr;
    } DrawWork;

    class LineDrawer {
        public:
            LineDrawer(uint32_t id, uint32_t x, uint32_t y);
            ~LineDrawer();

            virtual void addWorkBlocking(DrawWork work);
            virtual void addWorkBlocking(DrawWork work, int32_t * buff);
            virtual uint32_t getWorkQueueSize();
            virtual bool isWorkQueueEmpty();

            virtual void threadWorker();
            virtual void blockMainThreadUntilTheQueueIsNotEmpty();
            virtual void memCopySquareInto(SquareDefinition sqDef, int32_t * buff);

        private:
            uint32_t id;
            bool keep_running = true;

            std::queue<DrawWork> work_queue;
            std::thread thd;
            sem_t sem_work_queue{1};
            sem_t sem_block_this_thread{0};
            sem_t sem_block_main_thread{1};

            DrawBuffer buff;
    };
}

#endif /* !defined(DRAW_WORKER_H) */
