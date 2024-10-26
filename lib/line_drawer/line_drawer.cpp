#include <iostream>

#include "line_drawer.h"
#include "triangle.h"

namespace SG {

    LineDrawer::LineDrawer(uint32_t id) {
        this->id = id;
        // init semaphores
        sem_init(&sem_work_queue, 0, 1);
        sem_init(&sem_block_this_thread, 0, 0);
        sem_init(&sem_block_main_thread, 0, 1);
        // start worker thread
        this->thd = std::thread([this] { threadWorker(); } );
    }

    LineDrawer::~LineDrawer() {
        std::clog << "Thread " << id << " getting destroyed" << std::endl;
        this->keep_running = false;

        sem_post(&sem_work_queue);
        sem_post(&sem_block_this_thread);
        sem_post(&sem_block_main_thread);

        sem_destroy(&sem_work_queue);
        sem_destroy(&sem_block_this_thread);
        sem_destroy(&sem_block_main_thread);

        this->thd.join();

        std::clog << "Destroyed threads " << id << std::endl;
    }

    void LineDrawer::addWorkBlocking(DrawWork work) {
        sem_wait(&sem_work_queue);
        work_queue.push(work);
        sem_post(&sem_work_queue);

        // unblock this thread
        sem_post(&sem_block_this_thread);
        // block main thread to access this thread's resources
        sem_wait(&sem_block_main_thread);
    }

    void LineDrawer::blockMainThreadUntilTheQueueIsNotEmpty() {
        sem_wait(&sem_block_main_thread);
        sem_post(&sem_block_main_thread);
    }

    uint32_t LineDrawer::getWorkQueueSize() {
        sem_wait(&sem_work_queue);
        uint32_t size = work_queue.size();
        sem_post(&sem_work_queue);
        return size;
    }

    bool LineDrawer::isWorkQueueEmpty() {
        sem_wait(&sem_work_queue);
        bool empty = work_queue.empty();
        sem_post(&sem_work_queue);
        return empty;
    }

    void LineDrawer::threadWorker() {
        while (keep_running) {
            sem_wait(&sem_block_this_thread);
            sem_wait(&sem_work_queue);
            while (!work_queue.empty()) {
                DrawWork w = work_queue.front();
                work_queue.pop();

                int32_t buf_offset;
                switch (w.workType) {
                    case Triangle: 
                        {
                            GM::Triangle * tr = (GM::Triangle *) w.obj;
                            for (int32_t y = w.start_line; y <= w.end_line; y++) {
                                for (int32_t x = w.left; x <= w.right; x++) {
                                    GM::Vertex point = {(double)x, (double)y, 0.0};
                                    buf_offset = y * w.buf->width + x;
                                    w.buf->map[buf_offset] = tr->pointInTriangle(point) 
                                        ? w.color 
                                        : w.bg_color;
                                }
                            }
                        }
                        break;

                    case Digit:
                        {
                            char * digit = (char *) w.obj;
                            uint32_t width = w.right - w.left;
                            uint32_t height = w.end_line - w.start_line;
                            for (int32_t y = 0; y < height; y++) {
                                for (int32_t x = 0; x < width; x++) {
                                    buf_offset = (w.start_line + y) * w.buf->width + (w.left + x);
                                    w.buf->map[buf_offset] = digit[y * width + x] 
                                        ? w.color 
                                        : w.bg_color;
                                }
                            }
                        }
                        break;
                }
            }
            sem_post(&sem_work_queue);
            sem_post(&sem_block_main_thread);
        }
    }
}
