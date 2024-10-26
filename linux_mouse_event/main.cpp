#include <iostream> 
#include <csignal>
#include <chrono>

#include <cxxopts.hpp>
#include <mouse_event_reader.hpp>


bool keep_running = true;
LinuxMouseEvent::MouseEventReader * mouse_event_reader;

/**
 *
 */
void clean_up() {
    delete mouse_event_reader;
}

/**
 *
 */
void sig_handler(int signo) {
    if (signo == SIGINT) {
        std::cerr << " - Received SIGINT, cleaning up." << std::endl;
        keep_running = false;

        clean_up();

        exit(0);
    }
}

/**
 *
 */
int main(int argc, char **argv) {
    // argument parser
    cxxopts::Options options("Linux Mouse Event Reader", "A Linux Mouse event reader example by Szilveszter Zsigmond.");
    options.add_options()
        ("mouse-input-device", "Mouse input device path. ls -alh /dev/input/by-id", cxxopts::value<std::string>()->default_value("/dev/input/event7"))
        ("h,help", "Prints this help message.");
        ;
    auto arguments = options.parse(argc, argv);
    if (arguments.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    // registar signal handler
    signal(SIGINT, sig_handler);

    std::string input_device = arguments["mouse-input-device"].as<std::string>();
    mouse_event_reader = new LinuxMouseEvent::MouseEventReader(input_device.c_str());
    int32_t ret = mouse_event_reader->openEventFile();
    if (ret) {
        exit(ret);
    }

    while (keep_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        auto pos = mouse_event_reader->getMousePosition();
        std::cout << "Position: " << pos.x << ":" << pos.y << std::endl;
    }

    clean_up();

    return 0;
}

