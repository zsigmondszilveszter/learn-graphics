#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/epoll.h>
#include <iostream>
#include <cerrno>

#include "mouse_event_reader.hpp"

namespace LinuxMouseEvent {
    /**
     *
     */
    MouseEventReader::MouseEventReader(const char * eventFileHandler) {
        this->_eventFileHandler = eventFileHandler;
    }


    /**
     *
     */
    MouseEventReader::MouseEventReader(const char * eventFileHandler, uint32_t max_x, uint32_t max_y) {
        this->_eventFileHandler = eventFileHandler;
        this->max_x = max_x;
        this->max_y = max_y;
    }

    /**
     *
     */
    MouseEventReader::~MouseEventReader(){
        std::clog << "Mouse event reader Thread getting destroyed" << std::endl;
        this->keep_running = false;

        sem_post(&sem_mouse_position);

        sem_destroy(&sem_mouse_position);

        if (this->thd.joinable()) {
            this->thd.join();
        }

        if (fd) {
            close(fd);
        }
        std::clog << "Mouse Input file descriptor closed." << std::endl;
    }

    /**
     *
     */
    int32_t MouseEventReader::openEventFile() {
        int32_t ret;

        std::clog << "Opening Input Event file (" << _eventFileHandler << ")" << std::endl;
        /* open the event input file */
        ret = open(_eventFileHandler, O_RDONLY | O_NONBLOCK);
        if (ret == -1) {
            goto out_return;
        }
        this->fd = ret;


        // init semaphores
        sem_init(&sem_mouse_position, 0, 1);
        // start reader thread
        this->thd = std::thread([this] { readerThread(); } );

        return 0;

out_return:
        if (ret) {
            std::clog << "opening the Event input file failed with error " << errno << std::endl;
        }

        return ret;
    }

    /**
     *
     */
    void MouseEventReader::readerThread() {
        struct input_event ie;
        struct epoll_event ee;
        ee.events = EPOLLIN;
        ee.data.fd = fd;

        int32_t epfd = epoll_create1(0);
        if (epfd == -1) {
            perror("epoll_create1");
            close(fd);
            return;
        }

        if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ee) == -1) {
            perror("epoll_ctl");
            close(fd);
            close(epfd);
            return;
        }

        while (keep_running) {
            struct epoll_event events[1];
            int n = epoll_wait(epfd, events, 1, 500); // the timeout is 500ms
            if (n == -1) {
                perror("epoll_wait");
                break;
            }

            if (events[0].events & EPOLLIN) {
                // Read the event from the file descriptor
                ssize_t bytesRead = read(fd, &ie, sizeof(struct input_event));

                if (sizeof(events) == -1) {
                    if (errno == EAGAIN) {
                        // No data available, non-blocking mode (this is okay, continue loop)
                        continue;
                    } else {
                        throw std::runtime_error("Error reading mouse event");
                    }
                } else if (bytesRead == sizeof(struct input_event)) {
                    // Process the event type
                    switch (ie.type) {
                        case EV_KEY:
                            //std::cout << "Button " << (ie.code == BTN_LEFT ? "Left" : (ie.code == BTN_RIGHT ? "Right" : "Other"))
                            //    << " was " << (ie.value ? "pressed" : "released") << std::endl;
                            break;
                        case EV_REL:
                            if (ie.code == REL_X) {
                                movePosX(ie.value);
                            } else if (ie.code == REL_Y) {
                                movePosY(ie.value);
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        close(epfd);
    }

    void MouseEventReader::movePosX(int32_t x) {
        sem_wait(&sem_mouse_position);
        x_pos += x;
        sem_post(&sem_mouse_position);
        if (x_pos < 0) {
            x_pos = 0;
        }
        if (x_pos > max_x) {
            x_pos = max_x;
        }
    }

    void MouseEventReader::movePosY(int32_t y) {
        sem_wait(&sem_mouse_position);
        y_pos += y;
        sem_post(&sem_mouse_position);
        if (y_pos < 0) {
            y_pos = 0;
        }
        if (y_pos > max_y) {
            y_pos = max_y;
        }
    }

    /**
     * Reads the current mouse position.
     */
    MousePosition MouseEventReader::getMousePosition() {
        sem_wait(&sem_mouse_position);
        MousePosition pos = {
            (uint32_t) x_pos, 
            (uint32_t) y_pos
        };
        sem_post(&sem_mouse_position);
        return pos;
    }
}
