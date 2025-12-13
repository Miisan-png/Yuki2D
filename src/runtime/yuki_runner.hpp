#pragma once
#include <string>
#include "../core/window.hpp"

namespace yuki {

class YukiRunner {
public:
    YukiRunner(const std::string& scriptPath);
    YukiRunner(const std::string& scriptPath, bool watch);
    void run(Window& window);

private:
    std::string scriptPath;
    bool watch = false;
};

}
