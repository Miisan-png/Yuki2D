#include "dev_console.hpp"
#include "../core/renderer2d.hpp"
#include "../core/input.hpp"
#include "../script/token.hpp"
#include "../script/parser.hpp"
#include "../script/ast.hpp"
#include "../script/interpreter.hpp"
#include "../core/engine_bindings.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <memory>
namespace yuki {
namespace {
    std::string trim(const std::string& s) {
        size_t b = s.find_first_not_of(" \t\r\n");
        if (b == std::string::npos) return "";
        size_t e = s.find_last_not_of(" \t\r\n");
        return s.substr(b, e - b + 1);
    }
    constexpr float kConsoleFontScale = 2.0f;
}
DevConsole::DevConsole(Renderer2D* r, Interpreter* i) : active(false), renderer(r), interpreter(i), fontId(-1) {}
void DevConsole::setInterpreter(Interpreter* i) {
    interpreter = i;
}
void DevConsole::toggle() {
    active = !active;
    if (active) cursorPos = (int)input.size();
}
bool DevConsole::isActive() const {
    return active;
}
void DevConsole::appendLine(const std::string& line) {
    lines.push_back(line);
    if (lines.size() > 50) {
        lines.erase(lines.begin());
    }
    scrollOffset = 0;
}
void DevConsole::log(const std::string& line) {
    appendLine(line);
}
void DevConsole::submit() {
    if (!interpreter) return;
    std::string src = input;
    input.clear();
    cursorPos = 0;
    src = trim(src);
    if (src.empty()) return;
    if (src == "clear") {
        lines.clear();
        return;
    }
    if (src == "help") {
        appendLine("Console: type Yuki code and enter. Commands: help, clear.");
        appendLine("Tip: set globals like 'player_hp = 50;' then hit Enter.");
        return;
    }
    if (src == "globals" || src == "ls") {
        listGlobals();
        history.push_back(src);
        if (history.size() > 50) history.erase(history.begin());
        historyIndex = -1;
        return;
    }
    if (src.find(';') == std::string::npos && src.find('{') == std::string::npos) {
        src.push_back(';');
    }
    if (history.empty() || history.back() != src) {
        history.push_back(src);
        if (history.size() > 50) history.erase(history.begin());
    }
    historyIndex = -1;
    Tokenizer tokenizer(src);
    std::vector<Token> tokens = tokenizer.scanTokens();
    if (tokenizer.hadError()) {
        for (const auto& e : tokenizer.getErrors()) appendLine(e);
        return;
    }
    Parser parser(tokens);
    std::vector<std::unique_ptr<Stmt>> statements = parser.parse();
    if (parser.hadError()) {
        for (const auto& e : parser.getErrors()) appendLine(e);
        return;
    }
    interpreter->clearRuntimeErrors();
    std::unique_ptr<ExpressionStmt> lastExpr;
    if (!statements.empty() && statements.back()->getKind() == StmtKind::Expression) {
        std::unique_ptr<Stmt> last = std::move(statements.back());
        statements.pop_back();
        lastExpr.reset(static_cast<ExpressionStmt*>(last.release()));
    }
    interpreter->exec(statements);
    if (!statements.empty()) {
        interpreter->retainModule(std::move(statements));
    }
    if (interpreter->hasRuntimeErrors()) {
        for (const auto& e : interpreter->getRuntimeErrors()) appendLine(e);
        interpreter->clearRuntimeErrors();
        return;
    }
    appendLine("> " + src);
    if (lastExpr) {
        Value v = interpreter->evalExpr(lastExpr->expression.get());
        {
            std::vector<std::unique_ptr<Stmt>> keep;
            keep.push_back(std::unique_ptr<Stmt>(lastExpr.release()));
            interpreter->retainModule(std::move(keep));
        }
        if (v.type != ValueType::Nil) appendLine(v.toString());
        if (interpreter->hasRuntimeErrors()) {
            for (const auto& e : interpreter->getRuntimeErrors()) appendLine(e);
            interpreter->clearRuntimeErrors();
        }
    }
}
int DevConsole::ensureFont() {
    if (fontId >= 0) return fontId;
    if (!renderer) return -1;
    auto img = EngineBindings::resolveAssetPath("../assets/fonts/monogram_bitmap.png");
    auto metrics = EngineBindings::resolveAssetPath("../assets/fonts/monogram_bitmap.json");
    fontId = renderer->loadFont(img, metrics);
    return fontId;
}
void DevConsole::moveCursor(int delta) {
    cursorPos += delta;
    if (cursorPos < 0) cursorPos = 0;
    if (cursorPos > (int)input.size()) cursorPos = (int)input.size();
}
void DevConsole::insertChar(char c) {
    input.insert(input.begin() + cursorPos, c);
    cursorPos++;
}
void DevConsole::backspace() {
    if (cursorPos > 0 && !input.empty()) {
        input.erase(input.begin() + cursorPos - 1);
        cursorPos--;
    }
}
void DevConsole::listGlobals() {
    if (!interpreter || !interpreter->env) return;
    appendLine("Globals:");
    for (const auto& kv : interpreter->env->values) {
        std::string typeStr;
        switch (kv.second.type) {
            case ValueType::Nil: typeStr = "nil"; break;
            case ValueType::Number: typeStr = "number"; break;
            case ValueType::Bool: typeStr = "bool"; break;
            case ValueType::String: typeStr = "string"; break;
            case ValueType::Function: typeStr = "function"; break;
            case ValueType::Map: typeStr = "map"; break;
            case ValueType::Array: typeStr = "array"; break;
        }
        appendLine(" - " + kv.first + " (" + typeStr + ")");
    }
}
void DevConsole::historyPrev() {
    if (history.empty()) return;
    if (historyIndex < 0) historyIndex = (int)history.size() - 1;
    else if (historyIndex > 0) historyIndex--;
    input = history[historyIndex];
    cursorPos = (int)input.size();
}
void DevConsole::historyNext() {
    if (history.empty()) return;
    if (historyIndex >= 0 && historyIndex < (int)history.size() - 1) {
        historyIndex++;
        input = history[historyIndex];
    } else {
        historyIndex = -1;
        input.clear();
    }
    cursorPos = (int)input.size();
}
void DevConsole::scroll(int delta) {
    int maxOffset = (int)lines.size();
    scrollOffset += delta;
    if (scrollOffset < 0) scrollOffset = 0;
    if (scrollOffset > maxOffset) scrollOffset = maxOffset;
}
void DevConsole::updateInput() {
    if (isKeyPressed(GLFW_KEY_GRAVE_ACCENT) || isKeyPressed(GLFW_KEY_WORLD_1) || isKeyPressed(GLFW_KEY_F1) || isKeyPressed(GLFW_KEY_F2)) {
        toggle();
        return;
    }
    if (!active) return;
    bool shift = isKeyDown(GLFW_KEY_LEFT_SHIFT) || isKeyDown(GLFW_KEY_RIGHT_SHIFT);
    for (char c = 'a'; c <= 'z'; ++c) {
        int key = GLFW_KEY_A + (c - 'a');
        if (isKeyPressed(key)) insertChar(shift ? (char)std::toupper(c) : c);
    }
    static const char* shiftedDigits = ")!@#$%^&*(";
    for (char c = '0'; c <= '9'; ++c) {
        int key = GLFW_KEY_0 + (c - '0');
        if (isKeyPressed(key)) {
            if (shift) insertChar(shiftedDigits[c - '0']);
            else insertChar(c);
        }
    }
    if (isKeyPressed(GLFW_KEY_SPACE)) insertChar(' ');
    if (isKeyPressed(GLFW_KEY_PERIOD)) insertChar(shift ? '>' : '.');
    if (isKeyPressed(GLFW_KEY_COMMA)) insertChar(shift ? '<' : ',');
    if (isKeyPressed(GLFW_KEY_SLASH)) insertChar(shift ? '?' : '/');
    if (isKeyPressed(GLFW_KEY_APOSTROPHE)) insertChar(shift ? '\"' : '\'');
    if (isKeyPressed(GLFW_KEY_SEMICOLON)) insertChar(shift ? ':' : ';');
    if (isKeyPressed(GLFW_KEY_EQUAL)) insertChar(shift ? '+' : '=');
    if (isKeyPressed(GLFW_KEY_KP_ADD)) insertChar('+');
    if (isKeyPressed(GLFW_KEY_MINUS)) insertChar(shift ? '_' : '-');
    if (isKeyPressed(GLFW_KEY_KP_SUBTRACT)) insertChar('-');
    if (isKeyPressed(GLFW_KEY_LEFT_BRACKET)) insertChar(shift ? '{' : '[');
    if (isKeyPressed(GLFW_KEY_RIGHT_BRACKET)) insertChar(shift ? '}' : ']');
    if (isKeyPressed(GLFW_KEY_BACKSLASH)) insertChar(shift ? '|' : '\\');
    if (isKeyPressed(GLFW_KEY_9) && shift) insertChar('(');
    if (isKeyPressed(GLFW_KEY_0) && shift) insertChar(')');
    if (isKeyPressed(GLFW_KEY_8) && shift) insertChar('*');
    if (isKeyPressed(GLFW_KEY_6) && shift) insertChar('^');
    if (isKeyPressed(GLFW_KEY_BACKSPACE)) {
        backspace();
    }
    if (isKeyPressed(GLFW_KEY_LEFT)) moveCursor(-1);
    if (isKeyPressed(GLFW_KEY_RIGHT)) moveCursor(1);
    if (isKeyPressed(GLFW_KEY_HOME)) cursorPos = 0;
    if (isKeyPressed(GLFW_KEY_END)) cursorPos = (int)input.size();
    if (isKeyPressed(GLFW_KEY_UP)) historyPrev();
    if (isKeyPressed(GLFW_KEY_DOWN)) historyNext();
    if (isKeyPressed(GLFW_KEY_PAGE_UP)) scroll(5);
    if (isKeyPressed(GLFW_KEY_PAGE_DOWN)) scroll(-5);
    if (isKeyPressed(GLFW_KEY_ENTER)) {
        submit();
    }
}
void DevConsole::drawOverlay(int screenWidth, int screenHeight) {
    if (!active || !renderer) return;
    int fid = ensureFont();
    if (fid < 0) return;
    float h = screenHeight * 0.45f;
    float y = screenHeight - h;
    renderer->drawRect(0, y, (float)screenWidth, h, 0.05f, 0.05f, 0.06f);
    float textY = y + 10.0f;
    float lineStep = 16.0f * kConsoleFontScale;
    int maxLines = (int)((h - 30.0f) / lineStep);
    int start = 0;
    if ((int)lines.size() > maxLines) {
        start = (int)lines.size() - maxLines - scrollOffset;
        if (start < 0) start = 0;
    }
    int end = std::min((int)lines.size(), start + maxLines);
    for (int i = start; i < end; ++i) {
        renderer->drawTextEx(fid, lines[i], 8.0f, textY, kConsoleFontScale, 1.0f, 1.0f, 1.0f, 1.0f, "left", 0.0f, 0.0f);
        textY += lineStep;
    }
    std::string promptText = "> " + input;
    float promptY = y + h - 24.0f;
    renderer->drawTextEx(fid, promptText, 8.0f, promptY, kConsoleFontScale, 1.0f, 0.9f, 0.6f, 1.0f, "left", 0.0f, 0.0f);
    std::string caretText = "> " + input.substr(0, cursorPos);
    float caretX = 8.0f + renderer->measureTextWidth(fid, caretText, kConsoleFontScale, 0.0f, 0.0f);
    renderer->drawRect(caretX, promptY + 2.0f, 10.0f, 3.0f, 1.0f, 0.9f, 0.6f);
}
}
