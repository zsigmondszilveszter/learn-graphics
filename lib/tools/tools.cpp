#include "tools.hpp"
#include <thread>
#include <fstream>
#include <sstream>
#include <string>

namespace tl {
    uint32_t Tools::nr_of_cpus() {
        uint32_t cpu_count = std::thread::hardware_concurrency();
        if (cpu_count > 0) {
            return cpu_count;
        }
        std::ifstream cpuinfo("/proc/cpuinfo");
        std::string line;

        while (std::getline(cpuinfo, line)) {
            std::istringstream iss(line);
            std::string key, value;
            iss >> key >> value;

            if (key == "processor") {
                cpu_count++;
            }
        }
        cpuinfo.close();
        return cpu_count / 2;
    }
}
