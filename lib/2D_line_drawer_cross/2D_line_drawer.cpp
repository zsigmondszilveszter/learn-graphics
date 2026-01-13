#include <iostream>

#include "2D_line_drawer.hpp"

namespace szilv {

    LineDrawer2D::LineDrawer2D(uint32_t id, uint32_t x, uint32_t y) 
        : sem_work_queue(1), sem_block_this_thread(0), sem_block_main_thread(1) {
        this->id = id;
        
        // start worker thread
        this->thd = std::thread([this] { threadWorker(); } );
    }

    LineDrawer2D::~LineDrawer2D() {
        std::clog << "Thread " << id << " getting destroyed" << std::endl;
        this->keep_running = false;

        sem_work_queue.notify();
        sem_block_this_thread.notify();
        sem_block_main_thread.notify();

        this->thd.join();

        std::clog << "Destroyed threads " << id << std::endl;
    }

    void LineDrawer2D::addWorkBlocking(DrawWork work) {
        sem_work_queue.wait();
        work_queue.push(work);
        sem_work_queue.notify();

        // unblock this thread
        sem_block_this_thread.notify();
        // block main thread to access this thread's resources
        sem_block_main_thread.wait();
    }

    void LineDrawer2D::blockMainThreadUntilTheQueueIsNotEmpty() {
        sem_block_main_thread.wait();
        sem_block_main_thread.notify();
    }

    uint32_t LineDrawer2D::getWorkQueueSize() {
        sem_work_queue.wait();
        uint32_t size = work_queue.size();
        sem_work_queue.notify();
        return size;
    }

    bool LineDrawer2D::isWorkQueueEmpty() {
        sem_work_queue.wait();
        bool empty = work_queue.empty();
        sem_work_queue.notify();
        return empty;
    }

    void LineDrawer2D::threadWorker() {
        while (keep_running) {
            sem_block_this_thread.wait();
            sem_work_queue.wait();
            while (!work_queue.empty()) {
                DrawWork w = work_queue.front();
                work_queue.pop();

                int32_t buf_offset;
                for (int32_t y = w.squareDefinition.y1; y <= w.squareDefinition.y2; y++) {
                    for (int32_t x = w.squareDefinition.x1; x <= w.squareDefinition.x2; x++) {
                        szilv::Vertex point = {(double)x, (double)y, 0.0};
                        buf_offset = y * w.buff_width + x;
                        w.target_buff[buf_offset] = w.isInside(point) 
                            ? w.color 
                            : w.bg_color;
                    }
                }
            }
            sem_work_queue.notify();
            sem_block_main_thread.notify();
        }
    }
}