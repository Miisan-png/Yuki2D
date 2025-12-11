#pragma once
#include <string>
#include <vector>
namespace yuki {
class Renderer2D;
class Interpreter;
class DevConsole {
public:
    DevConsole(Renderer2D* renderer, Interpreter* interpreter);
    void toggle();
    bool isActive() const;
    void updateInput();
    void drawOverlay(int screenWidth, int screenHeight);
    void log(const std::string& line);
private:
    void appendLine(const std::string& line);
    void submit();
    int ensureFont();
    bool active;
    std::string input;
    std::vector<std::string> lines;
    Renderer2D* renderer;
    Interpreter* interpreter;
    int fontId;
};
}
