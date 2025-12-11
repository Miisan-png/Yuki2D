#include "dev_console.hpp"
#include "../core/renderer2d.hpp"
#include "../core/input.hpp"
#include "../script/token.hpp"
#include "../script/parser.hpp"
#include "../script/ast.hpp"
#include "../script/interpreter.hpp"
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
}
DevConsole::DevConsole(Renderer2D* r, Interpreter* i) : active(false), renderer(r), interpreter(i), fontId(-1) {}
void DevConsole::toggle() {
    active = !active;
}
bool DevConsole::isActive() const {
    return active;
}
void DevConsole::appendLine(const std::string& line) {
    lines.push_back(line);
    if (lines.size() > 50) {
        lines.erase(lines.begin());
    }
}
void DevConsole::log(const std::string& line) {
    appendLine(line);
}
void DevConsole::submit() {
    if (!interpreter) return;
    std::string src = input;
    input.clear();
    src = trim(src);
    if (src.empty()) return;
    Tokenizer tokenizer(src);
    std::vector<Token> tokens = tokenizer.scanTokens();
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
    if (interpreter->hasRuntimeErrors()) {
        for (const auto& e : interpreter->getRuntimeErrors()) appendLine(e);
        interpreter->clearRuntimeErrors();
        return;
    }
    appendLine("> " + src);
    if (lastExpr) {
        Value v = interpreter->evalExpr(lastExpr->expression.get());
        if (v.type != ValueType::Nil) appendLine(v.toString());
    }
}
int DevConsole::ensureFont() {
    if (fontId >= 0) return fontId;
    if (!renderer) return -1;
    fontId = renderer->loadFont("../assets/fonts/monogram_bitmap.png", "../assets/fonts/monogram_bitmap.json");
    return fontId;
}
void DevConsole::updateInput() {
    if (isKeyPressed(GLFW_KEY_GRAVE_ACCENT)) {
        toggle();
        return;
    }
    if (!active) return;
    for (char c = 'a'; c <= 'z'; ++c) {
        int key = GLFW_KEY_A + (c - 'a');
        if (isKeyPressed(key)) input.push_back(c);
    }
    for (char c = '0'; c <= '9'; ++c) {
        int key = GLFW_KEY_0 + (c - '0');
        if (isKeyPressed(key)) input.push_back(c);
    }
    if (isKeyPressed(GLFW_KEY_SPACE)) input.push_back(' ');
    if (isKeyPressed(GLFW_KEY_MINUS)) input.push_back('-');
    if (isKeyPressed(GLFW_KEY_EQUAL)) input.push_back('=');
    if (isKeyPressed(GLFW_KEY_PERIOD)) input.push_back('.');
    if (isKeyPressed(GLFW_KEY_COMMA)) input.push_back(',');
    if (isKeyPressed(GLFW_KEY_SLASH)) input.push_back('/');
    if (isKeyPressed(GLFW_KEY_BACKSPACE)) {
        if (!input.empty()) input.pop_back();
    }
    if (isKeyPressed(GLFW_KEY_ENTER)) {
        submit();
    }
}
void DevConsole::drawOverlay(int screenWidth, int screenHeight) {
    if (!active || !renderer) return;
    int fid = ensureFont();
    if (fid < 0) return;
    float h = screenHeight * 0.35f;
    float y = screenHeight - h;
    renderer->drawRect(0, y, (float)screenWidth, h, 0.05f, 0.05f, 0.06f);
    float textY = y + 8.0f;
    for (size_t i = 0; i < lines.size(); ++i) {
        renderer->drawTextEx(fid, lines[i], 8.0f, textY, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, "left", 0.0f, 0.0f);
        textY += 14.0f;
        if (textY > y + h - 30.0f) break;
    }
    std::string prompt = "> " + input + "_";
    renderer->drawTextEx(fid, prompt, 8.0f, y + h - 20.0f, 1.0f, 1.0f, 0.9f, 0.6f, 1.0f, "left", 0.0f, 0.0f);
}
}
