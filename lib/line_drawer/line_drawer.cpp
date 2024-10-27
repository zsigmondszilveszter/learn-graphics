#include <iostream>

#include "line_drawer.h"
#include "triangle.h"
#include <cstdlib> 
#include <cstring>

namespace SG {

    LineDrawer::LineDrawer(uint32_t id, uint32_t x, uint32_t y) {
        this->id = id;
        // init semaphores
        sem_init(&sem_work_queue, 0, 1);
        sem_init(&sem_block_this_thread, 0, 0);
        sem_init(&sem_block_main_thread, 0, 1);

        // init the worker's own buffer
        this->buff.width = x;
        this->buff.height = y;
        this->buff.map = (int32_t *) std::malloc(x * y * sizeof(int32_t));

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

        free(this->buff.map);

        std::clog << "Destroyed threads " << id << std::endl;
    }

    void LineDrawer::addWorkBlocking(DrawWork work) {
        addWorkBlocking(work, this->buff.map);
    }

    void LineDrawer::addWorkBlocking(DrawWork work, int32_t * buff) {
        work.buff = buff;
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

    void LineDrawer::memCopySquareInto(SquareDefinition sqDef, int32_t * buff) {
        sem_wait(&sem_work_queue);

        int32_t size = sizeof(int32_t) * (sqDef.x2 - sqDef.x1 + 1);
        for (int32_t y = sqDef.y1; y <= sqDef.y2; y++) {
            int32_t offset = y * this->buff.width + sqDef.x1;
            std::memcpy(buff + offset, this->buff.map + offset, size);
        }

        sem_post(&sem_work_queue);
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
                            for (int32_t y = w.squareDefinition.y1; y <= w.squareDefinition.y2; y++) {
                                for (int32_t x = w.squareDefinition.x1; x <= w.squareDefinition.x2; x++) {
                                    GM::Vertex point = {(double)x, (double)y, 0.0};
                                    buf_offset = y * buff.width + x;
                                    w.buff[buf_offset] = tr->pointInTriangle(point) 
                                        ? w.color 
                                        : w.bg_color;
                                }
                            }
                        }
                        break;

                    case Digit:
                        {
                            char * digit = (char *) w.obj;
                            uint32_t width = w.squareDefinition.x2 - w.squareDefinition.x1;
                            uint32_t height = w.squareDefinition.y2 - w.squareDefinition.y1;
                            for (int32_t y = 0; y < height; y++) {
                                for (int32_t x = 0; x < width; x++) {
                                    buf_offset = (w.squareDefinition.y1 + y) * buff.width + (w.squareDefinition.x1 + x);
                                    w.buff[buf_offset] = digit[y * width + x] 
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
