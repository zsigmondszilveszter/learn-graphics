#include <iostream>

#include "2D_line_drawer.hpp"
#include <cstdlib> 
#include <cstring>

namespace szilv {

    LineDrawer2D::LineDrawer2D(uint32_t id, uint32_t x, uint32_t y) {
        this->id = id;
        // init semaphores
        sem_init(&sem_work_queue, 0, 1);
        sem_init(&sem_block_this_thread, 0, 0);
        sem_init(&sem_block_main_thread, 0, 1);

        // start worker thread
        this->thd = std::thread([this] { threadWorker(); } );
    }

    LineDrawer2D::~LineDrawer2D() {
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

    void LineDrawer2D::addWorkBlocking(DrawWork work) {
        sem_wait(&sem_work_queue);
        work_queue.push(work);
        sem_post(&sem_work_queue);

        // unblock this thread
        sem_post(&sem_block_this_thread);
        // block main thread to access this thread's resources
        sem_wait(&sem_block_main_thread);
    }

    void LineDrawer2D::blockMainThreadUntilTheQueueIsNotEmpty() {
        sem_wait(&sem_block_main_thread);
        sem_post(&sem_block_main_thread);
    }

    uint32_t LineDrawer2D::getWorkQueueSize() {
        sem_wait(&sem_work_queue);
        uint32_t size = work_queue.size();
        sem_post(&sem_work_queue);
        return size;
    }

    bool LineDrawer2D::isWorkQueueEmpty() {
        sem_wait(&sem_work_queue);
        bool empty = work_queue.empty();
        sem_post(&sem_work_queue);
        return empty;
    }

    void LineDrawer2D::threadWorker() {
        while (keep_running) {
            sem_wait(&sem_block_this_thread);
            sem_wait(&sem_work_queue);
            while (!work_queue.empty()) {
                DrawWork w = work_queue.front();
                work_queue.pop();

                int32_t buf_offset;
                //szilv::Triangle2D * tr = (szilv::Triangle2D *) w.obj;
                for (int32_t y = w.squareDefinition.y1; y <= w.squareDefinition.y2; y++) {
                    for (int32_t x = w.squareDefinition.x1; x <= w.squareDefinition.x2; x++) {
                        szilv::Vertex point = {(double)x, (double)y, 0.0};
                        buf_offset = y * w.buff_width + x;
                        w.target_buff[buf_offset] = w.isInside(point) 
                            ? w.color 
                            : w.bg_color;
                    }
                }
                    //case Digit:
                    //    {
                    //        char * digit = (char *) w.obj;
                    //        uint32_t width = w.squareDefinition.x2 - w.squareDefinition.x1;
                    //        uint32_t height = w.squareDefinition.y2 - w.squareDefinition.y1;
                    //        for (int32_t y = 0; y < height; y++) {
                    //            for (int32_t x = 0; x < width; x++) {
                    //                buf_offset = (w.squareDefinition.y1 + y) * buff.width + (w.squareDefinition.x1 + x);
                    //                w.target_buff[buf_offset] = digit[y * width + x] 
                    //                    ? w.color 
                    //                    : w.bg_color;
                    //            }
                    //        }
                    //    }
                    //    break;
                //}
            }
            sem_post(&sem_work_queue);
            sem_post(&sem_block_main_thread);
        }
    }
}
