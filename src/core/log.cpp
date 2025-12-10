#include "log.hpp"
#include <iostream>
namespace yuki {
void logInfo(const std::string& msg) {
    std::cout << "[INFO] " << msg << std::endl;
}
void logError(const std::string& msg) {
    std::cerr << "[ERROR] " << msg << std::endl;
}
}
