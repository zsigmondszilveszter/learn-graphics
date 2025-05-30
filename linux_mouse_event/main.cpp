#include <iostream> 
#include <csignal>
#include <chrono>

#include "cli_args_szilv.hpp"
#include "mouse_event_reader.hpp"


bool keep_running = true;
szcl::MouseEventReader * mouse_event_reader;

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
    szcl::CliArgsSzilv cliArgs("Linux Mouse Event Reader", "A Linux Mouse event reader example by Szilveszter Zsigmond.");

    try {
        cliArgs.addOptionString("m,mouse-input-device", "Mouse input device path. ls -alh /dev/input/by-id");
        cliArgs.addOptionHelp("h,help", "Prints this help message.");
        cliArgs.parseArguments(argc, argv);
    } catch (szcl::CliArgsSzilvException& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }
    if (cliArgs.isHelp()) {
        std::cout << cliArgs.getHelpDisplay() << std::endl;
        return 0;
    }

    // registar signal handler
    signal(SIGINT, sig_handler);

    std::string input_device = cliArgs.getOptionString("mouse-input-device");
    mouse_event_reader = new szcl::MouseEventReader(input_device.c_str(), 1024, 768, 512, 384);
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

