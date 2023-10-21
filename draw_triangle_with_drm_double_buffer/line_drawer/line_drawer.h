#if !defined(DRAW_WORKER_H)
#define DRAW_WORKER_H

#include <queue>
#include <thread>
//#include <semaphore>
#include <semaphore.h> 

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

    typedef sem_t sz_semaphore;
    //typedef std::binary_semaphore sz_semaphore;

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

            std::queue<DrawWork> work_queue;
            std::thread thd;

            sz_semaphore sem1;
            sz_semaphore sem2;
            sz_semaphore sem3;
            sz_semaphore * sem_work_queue = &sem1;
            sz_semaphore * sem_block_this_thread = &sem2;
            sz_semaphore * sem_block_main_thread = &sem3;

            void initSemaphore(sz_semaphore * sem, uint32_t initialValue) {
                sem_init(sem, 0, initialValue);
            }
            void acquireSemaphore(sz_semaphore * sem) {
                sem_wait(sem);
            }
            void releaseSemaphore(sz_semaphore * sem) {
                sem_post(sem);
            }
            void destroySemaphore(sz_semaphore * sem) {
                sem_destroy(sem);
            }

            //sz_semaphore sem1{1};
            //sz_semaphore sem2{0};
            //sz_semaphore sem3{1};
            //sz_semaphore * sem_work_queue = &sem1;
            //sz_semaphore * sem_block_this_thread = &sem2;
            //sz_semaphore * sem_block_main_thread = &sem3;

            //void initSemaphore(sz_semaphore * sem, uint32_t initialValue) {}
            //void acquireSemaphore(sz_semaphore * sem) {
            //    sem->acquire();
            //}
            //void releaseSemaphore(sz_semaphore * sem) {
            //    sem->release();
            //}
            //void destroySemaphore(sz_semaphore * sem) {}

    };
}

#endif /* !defined(DRAW_WORKER_H) */
