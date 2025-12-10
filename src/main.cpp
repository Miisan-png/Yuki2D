#include "window.hpp"
#include "yuki_runner.hpp"
#include "log.hpp"
#include "config.hpp"
#include <string>
int main(int argc, char** argv) {
    std::string scriptPath = "scripts/example_main.ys";
    if (argc > 1) {
        scriptPath = argv[1];
    }
    yuki::logInfo("Engine starting");
    yuki::logInfo("Using script: " + scriptPath);
    yuki::logInfo("Time system initialized");
    yuki::EngineConfig config;
    yuki::Window window(config.width, config.height, config.title);
    yuki::YukiRunner runner(scriptPath);
    runner.run(window);
    return 0;
}