#if !defined(MOUSE_EVENT_READER_H)
#define MOUSE_EVENT_READER_H

#include <cstdint>
#include <thread>
#include <semaphore.h>

namespace LinuxMouseEvent {

    typedef struct {
        uint32_t x;
        uint32_t y;
    } MousePosition;

    class MouseEventReader {
        public:
            MouseEventReader(const char * eventFileHandler);
            ~MouseEventReader();
            virtual int32_t openEventFile();
            virtual MousePosition getMousePosition();
        private:
            const char * _eventFileHandler;
            int32_t fd = 0;
            bool keep_running = true;

            sem_t sem_mouse_position{1};
            int32_t x_pos = 0; 
            int32_t y_pos = 0;
            std::thread thd;

            virtual void readerThread();
            virtual void movePosX(int32_t x);
            virtual void movePosY(int32_t y);
    };
}
#endif /* !defined(MOUSE_EVENT_READER_H) */
