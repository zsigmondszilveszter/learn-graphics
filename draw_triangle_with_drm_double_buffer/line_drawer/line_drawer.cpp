#include <iostream>
#include "unistd.h"

#include "line_drawer.h"
#include "triangle.h"

namespace SG {

    LineDrawer::LineDrawer(uint32_t id) {
        this->id = id;
        this->thd = std::thread([this] { threadWorker(); } );
    }

    LineDrawer::~LineDrawer() {
        std::clog << "Thread " << id << " getting destroyed" << std::endl;
        this->keep_running = false;

        sem_work_queue.release();
        sem_block_this_thread.release();
        sem_block_main_thread.release();

        this->thd.join();

        std::clog << "Destroyed threads " << id << std::endl;
    }

    void LineDrawer::addWorkBlocking(DrawWork work) {
        sem_work_queue.acquire();
//        work_queue.push(work);
        sem_work_queue.release();

        // unblock this thread
        sem_block_this_thread.release();
        // block main thread to access this thread's resources
        sem_block_main_thread.acquire();
    }

    bool LineDrawer::addWorkNonblocking(DrawWork work) {
        if (sem_work_queue.try_acquire()) {
            work_queue.push(work);
            sem_work_queue.release();

            sem_block_this_thread.release();
            sem_block_main_thread.acquire();
            return true;
        }
        return false;
    }

    void LineDrawer::blockMainThreadUntilTheQueueIsNotEmpty() {
        sem_block_main_thread.acquire();
        sem_block_main_thread.release();
    }

    uint32_t LineDrawer::getWorkQueueSize() {
        sem_work_queue.acquire();
 //       uint32_t size = work_queue.size();
        sem_work_queue.release();
 //       return size;
        return 0;
    }

    bool LineDrawer::isWorkQueueEmpty() {
        sem_work_queue.acquire();
        //bool empty = work_queue.empty();
        sem_work_queue.release();
        //return empty;
        return true;
    }

    void LineDrawer::threadWorker() {
        while (keep_running) {
            sem_block_this_thread.acquire();
            sem_work_queue.acquire();
            //usleep(1);
            //while (!work_queue.empty()) {
            //    DrawWork w = work_queue.front();
            //    work_queue.pop();

            //    int32_t buf_offset;
            //    switch (w.workType) {
            //        case Triangle: 
            //            {
            //                GM::Triangle * tr = (GM::Triangle *) w.obj;
            //                for (int32_t y = w.start_line; y <= w.end_line; y++) {
            //                    for (int32_t x = w.left; x <= w.right; x++) {
            //                        GM::Vertex point = {(double)x, (double)y, 0.0};
            //                        buf_offset = y * w.buf->width + x;
            //                        w.buf->map[buf_offset] = tr->pointInTriangle(point) 
            //                            ? w.color 
            //                            : w.bg_color;
            //                    }
            //                }
            //            }
            //            break;

            //        case Digit:
            //            {
            //                char * digit = (char *) w.obj;
            //                uint32_t width = w.right - w.left;
            //                uint32_t height = w.end_line - w.start_line;
            //                for (int32_t y = 0; y < height; y++) {
            //                    for (int32_t x = 0; x < width; x++) {
            //                        buf_offset = (w.start_line + y) * w.buf->width + (w.left + x);
            //                        w.buf->map[buf_offset] = digit[y * width + x] 
            //                            ? w.color 
            //                            : w.bg_color;
            //                    }
            //                }
            //            }
            //            break;
            //    }
            //}
            sem_work_queue.release();
            sem_block_main_thread.release();
        }
    }
}
