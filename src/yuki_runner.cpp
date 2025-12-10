#include "yuki_runner.hpp"
#include "yuki_script_loader.hpp"
#include "log.hpp"
#include "time.hpp"
#include "input.hpp"
#include <string>
#include <GLFW/glfw3.h>
namespace yuki {
YukiRunner::YukiRunner(const std::string& scriptPath) : scriptPath(scriptPath) {}
void YukiRunner::run(Window& window) {
    logInfo("Running script: " + scriptPath);
    ScriptLoader loader(scriptPath);
    std::string content = loader.load();
    logInfo("Loaded script: " + std::to_string(content.size()) + " bytes");
    Time time;
    initInput(window);
    while (!window.shouldClose()) {
        time.update();
        [[maybe_unused]] float dt = time.deltaTime();
        updateInput();
        if (isKeyPressed(GLFW_KEY_SPACE)) {
            logInfo("Space pressed");
        }
        window.clear(0.0f, 0.0f, 0.5f, 1.0f);
        window.swapBuffers();
        window.pollEvents();
    }
}
}
