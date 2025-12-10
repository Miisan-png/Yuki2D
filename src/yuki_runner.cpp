#include "yuki_runner.hpp"
#include "log.hpp"
namespace yuki {
YukiRunner::YukiRunner(const std::string& scriptPath) : scriptPath(scriptPath) {}
void YukiRunner::run(Window& window) {
    logInfo("Running script: " + scriptPath);
    while (!window.shouldClose()) {
        window.clear(0.0f, 0.0f, 0.5f, 1.0f);
        window.swapBuffers();
        window.pollEvents();
    }
}
}