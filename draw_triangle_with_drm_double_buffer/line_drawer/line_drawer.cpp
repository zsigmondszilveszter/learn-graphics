#include "line_drawer.h"
#include <iostream>

namespace Szilv {

    LineDrawer::LineDrawer(uint32_t id) {
        this->id = id;
        this->triangleThd = std::thread([this] { triangleWorker(); } );
        this->fpsThd = std::thread([this] { fpsWorker(); } );
    }

    LineDrawer::~LineDrawer() {
        std::clog << "Thread " << id << " getting destroyed" << std::endl;
        this->keep_running = false;

        triangle_work_queue_sem.release();
        triangle_thread_sem.release();

        fps_work_queue_sem.release();
        fps_thread_sem.release();

        this->triangleThd.join();
        this->fpsThd.join();

        std::clog << "Destroyed threads " << id << std::endl;
    }

    void LineDrawer::addTriangleWorkBlocking(TriangleDrawWork work) {
        triangle_work_queue_sem.acquire();
        triangle_work_queue.push(work);
        triangle_work_queue_sem.release();
        triangle_thread_sem.release();
    }

    bool LineDrawer::addTriangleWorkNonblocking(TriangleDrawWork work) {
        if (triangle_work_queue_sem.try_acquire()) {
            triangle_work_queue.push(work);
            triangle_work_queue_sem.release();
            triangle_thread_sem.release();
            return true;
        }
        return false;
    }

    void LineDrawer::addFpsWorkBlocking(FpsDrawWork work) {
        fps_work_queue_sem.acquire();
        fps_work_queue.push(work);
        fps_work_queue_sem.release();
        fps_thread_sem.release();
    }

    bool LineDrawer::addFpsWorkNonblocking(FpsDrawWork work) {
        if (fps_work_queue_sem.try_acquire()) {
            fps_work_queue.push(work);
            fps_work_queue_sem.release();
            fps_thread_sem.release();
            return true;
        }
        return false;
    }

    uint32_t LineDrawer::getTriangleWorkQueueSize() {
        triangle_work_queue_sem.acquire();
        uint32_t size = triangle_work_queue.size();
        triangle_work_queue_sem.release();
        return size;
    }

    bool LineDrawer::isTriangleWorkQueueEmpty() {
        triangle_work_queue_sem.acquire();
        bool empty = triangle_work_queue.empty();
        triangle_work_queue_sem.release();
        return empty;
    }

    uint32_t LineDrawer::getFpsWorkQueueSize() {
        fps_work_queue_sem.acquire();
        uint32_t size = fps_work_queue.size();
        fps_work_queue_sem.release();
        return size;
    }

    bool LineDrawer::isFpsWorkQueueEmpty() {
        fps_work_queue_sem.acquire();
        bool empty = fps_work_queue.empty();
        fps_work_queue_sem.release();
        return empty;
    }

    void LineDrawer::executeTriangleDrawWork() {
        triangle_work_queue_sem.acquire();
        while (!triangle_work_queue.empty()) {
            TriangleDrawWork w = triangle_work_queue.front();
            triangle_work_queue.pop();
            for (int32_t y = w.start_line; y <= w.end_line; y++) {
                for (int32_t x = w.left; x <= w.right; x++) {
                    GM::Vertex point = {(double)x, (double)y, 0.0};
                    int32_t buf_offset = y * w.buf->width + x;
                    w.buf->map[buf_offset] = w.tr->pointInTriangle(point) ? w.color : w.bg_color;
                }
            }
        }
        triangle_work_queue_sem.release();
    }

    void LineDrawer::executeFpsDigitDrawWork() {
        fps_work_queue_sem.acquire();
        while (!fps_work_queue.empty()) {
            FpsDrawWork w = fps_work_queue.front();
            fps_work_queue.pop();
            uint32_t width = w.right - w.left;
            uint32_t height = w.end_line - w.start_line;
            for (int32_t y = 0; y < height; y++) {
                for (int32_t x = 0; x < width; x++) {
                    int32_t buf_offset = (w.start_line + y) * w.buf->width + (w.left + x);
                    w.buf->map[buf_offset] = w.digit[y * width + x] ? w.color : w.bg_color;
                }
            }
        }
        fps_work_queue_sem.release();
    }

    void LineDrawer::triangleWorker() {
        while (keep_running) {
            triangle_thread_sem.acquire();
            executeTriangleDrawWork();
        }
    }

    void LineDrawer::fpsWorker() {
        while (keep_running) {
            fps_thread_sem.acquire();
            executeFpsDigitDrawWork();
        }
    }
}
