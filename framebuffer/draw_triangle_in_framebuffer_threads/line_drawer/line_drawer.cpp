#include "line_drawer.h"
#include <iostream>

namespace Szilv {

    LineDrawer::LineDrawer(uint32_t id, int32_t w, int32_t h, uint32_t * framebuffer) {
        this->id = id;
        this->fb_width = w;
        this->fb_height = h;
        this->fbdata = framebuffer;

        this->thd = std::thread([this] { worker(); } );
    }

    LineDrawer::~LineDrawer() {
        std::clog << "Thread " << id << " getting destroyed" << std::endl;
        this->keep_running = false;
        triangle_work_queue_sem.release();
        thread_sem.release();
        this->thd.join();
        std::clog << "Destroyed thread " << id << std::endl;
    }

    void LineDrawer::addWorkBlocking(TriangleDrawWorkFrameBuffer work) {
        triangle_work_queue_sem.acquire();
        triangle_work_queue.push(work);
        triangle_work_queue_sem.release();
        thread_sem.release();
    }

    bool LineDrawer::addWorkNonblocking(TriangleDrawWorkFrameBuffer work) {
        if (triangle_work_queue_sem.try_acquire()) {
            triangle_work_queue.push(work);
            triangle_work_queue_sem.release();
            return true;
        }
        return false;
    }

    uint32_t LineDrawer::getWorkQueueSize() {
        triangle_work_queue_sem.acquire();
        uint32_t size = triangle_work_queue.size();
        triangle_work_queue_sem.release();
        return size;
    }

    bool LineDrawer::isEmpty() {
        triangle_work_queue_sem.acquire();
        bool empty = triangle_work_queue.empty();
        triangle_work_queue_sem.release();
        return empty;
    }

    void LineDrawer::executeTriangleDrawWorkInFrameBuffer() {
        triangle_work_queue_sem.acquire();
        while (!triangle_work_queue.empty()) {
            TriangleDrawWorkFrameBuffer w = triangle_work_queue.front();
            triangle_work_queue.pop();
            for (int32_t y = w.start_line; y <= w.end_line; y++) {
                for (int32_t x = w.left; x <= w.right; x++) {
                    GM::Vertex point = {(double)x, (double)y, 0.0};
                    int32_t buf_offset = y * fb_width + x;
                    fbdata[buf_offset] = w.tr->pointInTriangle(point) ? w.color : w.bg_color;
                }
            }
        }
        triangle_work_queue_sem.release();
    }

    void LineDrawer::worker() {
        while (keep_running) {
            thread_sem.acquire();
            executeTriangleDrawWorkInFrameBuffer();
        }
    }
}
