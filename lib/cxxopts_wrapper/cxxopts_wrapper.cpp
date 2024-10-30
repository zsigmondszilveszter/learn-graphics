#include "cxxopts_wrapper.hpp"

#include <cxxopts.hpp>

namespace tl {
    cxxopts::Options * opts;
    cxxopts::ParseResult res;

    CxxOptsWrapper::CxxOptsWrapper(const std::string &usage, const std::string &description) {
        opts = new cxxopts::Options(usage, description);
    }
    CxxOptsWrapper::~CxxOptsWrapper() {
        delete opts;
    }

    void CxxOptsWrapper::addOptionHelp(const std::string &description) {
        opts->add_options()
            ("h,help", description);
    }

    std::string CxxOptsWrapper::getHelp() {
        return opts->help();
    }

    void CxxOptsWrapper::addOptionBoolean(const std::string &group, const std::string &description) {
        opts->add_options()
            (group, description);
    }

    void CxxOptsWrapper::addOptionString(const std::string &group, const std::string &description, const std::string &defaultValue) {
        opts->add_options()
            (group, description, cxxopts::value<std::string>()->default_value(defaultValue));
    }

    void CxxOptsWrapper::addOptionInteger(const std::string &group, const std::string &description) {
        opts->add_options()
            (group, description, cxxopts::value<uint32_t>());
    }

    void CxxOptsWrapper::parseArguments(int &argc, char **argv) {
        res = opts->parse(argc, argv);
    }

    uint32_t CxxOptsWrapper::count(const std::string &name) {
        return res.count(name);
    }
    
    uint32_t CxxOptsWrapper::getOptionInteger(const std::string &name) {
        return res[name].as<uint32_t>();
    }

    std::string CxxOptsWrapper::getOptionString(const std::string &name) {
        return res[name].as<std::string>();
    }

    bool CxxOptsWrapper::getOptionBoolean(const std::string &name) {
        return res[name].as<bool>();
    }
}
