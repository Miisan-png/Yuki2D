#include "yuki_script_loader.hpp"
#include <fstream>
#include <sstream>
namespace yuki {
ScriptLoader::ScriptLoader(const std::string& path) : path(path) {}
std::string ScriptLoader::load() {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
}
