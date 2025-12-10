#pragma once
#include <string>
#include "../core/window.hpp"

namespace yuki {

class YukiRunner {
public:
    YukiRunner(const std::string& scriptPath);
    void run(Window& window);

private:
    std::string scriptPath;
};

}