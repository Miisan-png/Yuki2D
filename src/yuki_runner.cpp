#include "yuki_runner.hpp"
#include <iostream>
namespace yuki {
YukiRunner::YukiRunner(const std::string& scriptPath) : scriptPath(scriptPath) {}
void YukiRunner::run(Window& window) {
    std::cout << "Running script: " << scriptPath << std::endl;
    while (!window.shouldClose()) {
        window.clear(0.0f, 0.0f, 0.5f, 1.0f);
        window.swapBuffers();
        window.pollEvents();
    }
}
}
