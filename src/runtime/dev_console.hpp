#pragma once
#include <string>
#include <vector>
namespace yuki {
class Renderer2D;
class Interpreter;
class DevConsole {
public:
    DevConsole(Renderer2D* renderer, Interpreter* interpreter);
    void setInterpreter(Interpreter* interpreter);
    void toggle();
    bool isActive() const;
    void updateInput();
    void drawOverlay(int screenWidth, int screenHeight);
    void log(const std::string& line);
private:
    void appendLine(const std::string& line);
    void submit();
    int ensureFont();
    void moveCursor(int delta);
    void insertChar(char c);
    void backspace();
    void historyPrev();
    void historyNext();
    void listGlobals();
    void scroll(int delta);
    bool active;
    std::string input;
    int cursorPos = 0;
    std::vector<std::string> history;
    int historyIndex = -1;
    std::vector<std::string> lines;
    int scrollOffset = 0;
    Renderer2D* renderer;
    Interpreter* interpreter;
    int fontId;
};
}
