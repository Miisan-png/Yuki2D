#include "window.hpp"
#include "yuki_runner.hpp"
#include "log.hpp"
#include <string>
int main(int argc, char** argv) {
    std::string scriptPath = "scripts/example_main.ys";
    if (argc > 1) {
        scriptPath = argv[1];
    }
    yuki::logInfo("Engine starting");
    yuki::logInfo("Using script: " + scriptPath);
    yuki::logInfo("Time system initialized");
    yuki::Window window(1280, 720, "Yuki2D");
    yuki::YukiRunner runner(scriptPath);
    runner.run(window);
    return 0;
}
