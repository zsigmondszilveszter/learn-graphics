#if !defined(CXXOPTS_WRAPPER)
#define CXXOPTS_WRAPPER

#include <string>
#include <cstdint>

namespace tl {
    class CxxOptsWrapper {
        public:
            CxxOptsWrapper(const std::string &usage, const std::string &description);
            ~CxxOptsWrapper();

            void addOptionHelp(const std::string &description);
            std::string getHelp();
            void addOptionBoolean(const std::string &group, const std::string &description);
            void addOptionString(const std::string &group, const std::string &description, const std::string &defaultValue);
            void addOptionInteger(const std::string &group, const std::string &description);
            void parseArguments(int &argc, char **argv);
            uint32_t count(const std::string &name);
            uint32_t getOptionInteger(const std::string &name);
            std::string getOptionString(const std::string &name);
            bool getOptionBoolean(const std::string &name);
    };
}

#endif /* !defined(CXXOPTS_WRAPPER) */
