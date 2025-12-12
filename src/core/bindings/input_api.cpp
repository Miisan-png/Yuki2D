#include "input_api.hpp"
#include "state.hpp"
#include "../input.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>

namespace yuki {
namespace {
BindingsState& st = bindingsState();

std::unordered_map<std::string, int> buildKeyMap() {
    std::unordered_map<std::string, int> m;
    m["left"] = GLFW_KEY_LEFT;
    m["right"] = GLFW_KEY_RIGHT;
    m["up"] = GLFW_KEY_UP;
    m["down"] = GLFW_KEY_DOWN;
    m["space"] = GLFW_KEY_SPACE;
    m["enter"] = GLFW_KEY_ENTER;
    m["esc"] = GLFW_KEY_ESCAPE;
    m["escape"] = GLFW_KEY_ESCAPE;
    m["shift"] = GLFW_KEY_LEFT_SHIFT;
    m["ctrl"] = GLFW_KEY_LEFT_CONTROL;
    m["alt"] = GLFW_KEY_LEFT_ALT;
    for (char c = 'a'; c <= 'z'; ++c) {
        std::string s(1, c);
        m[s] = GLFW_KEY_A + (c - 'a');
    }
    for (char c = '0'; c <= '9'; ++c) {
        std::string s(1, c);
        m[s] = GLFW_KEY_0 + (c - '0');
    }
    return m;
}

std::unordered_map<std::string, int> buildMouseMap() {
    std::unordered_map<std::string, int> m;
    m["mouse_left"] = GLFW_MOUSE_BUTTON_LEFT;
    m["mouse_right"] = GLFW_MOUSE_BUTTON_RIGHT;
    m["mouse_middle"] = GLFW_MOUSE_BUTTON_MIDDLE;
    m["mouse_4"] = GLFW_MOUSE_BUTTON_4;
    m["mouse_5"] = GLFW_MOUSE_BUTTON_5;
    m["mouse_6"] = GLFW_MOUSE_BUTTON_6;
    m["mouse_7"] = GLFW_MOUSE_BUTTON_7;
    m["mouse_8"] = GLFW_MOUSE_BUTTON_8;
    return m;
}

const std::unordered_map<std::string, int> kKeyMap = buildKeyMap();
const std::unordered_map<std::string, int> kMouseMap = buildMouseMap();

int resolveKeyName(const Value& keyVal) {
    if (keyVal.isString()) {
        std::string name = keyVal.stringVal;
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return (char)std::tolower(c); });
        auto it = kKeyMap.find(name);
        if (it != kKeyMap.end()) return it->second;
        return -1;
    }
    if (keyVal.isNumber()) {
        return (int)keyVal.numberVal;
    }
    return -1;
}
std::pair<bool, int> resolveBinding(const Value& v) {
    if (v.isString()) {
        std::string name = v.stringVal;
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return (char)std::tolower(c); });
        auto keyIt = kKeyMap.find(name);
        if (keyIt != kKeyMap.end()) return {false, keyIt->second};
        auto mouseIt = kMouseMap.find(name);
        if (mouseIt != kMouseMap.end()) return {true, mouseIt->second};
        return {false, -1};
    }
    if (v.isNumber()) {
        return {false, (int)v.numberVal};
    }
    return {false, -1};
}
}

Value apiIsKeyDown(const std::vector<Value>& args) {
    if (args.empty()) return Value::boolean(false);
    int key = resolveKeyName(args[0]);
    if (key >= 0) {
        return Value::boolean(isKeyDown(key));
    }
    return Value::boolean(false);
}
Value apiIsKeyPressed(const std::vector<Value>& args) {
    if (args.empty()) return Value::boolean(false);
    int key = resolveKeyName(args[0]);
    if (key >= 0) {
        return Value::boolean(isKeyPressed(key));
    }
    return Value::boolean(false);
}
Value apiIsKeyReleased(const std::vector<Value>& args) {
    if (args.empty()) return Value::boolean(false);
    int key = resolveKeyName(args[0]);
    if (key >= 0) {
        return Value::boolean(isKeyReleased(key));
    }
    return Value::boolean(false);
}
Value apiIsMouseDown(const std::vector<Value>& args) {
    if (args.empty()) return Value::boolean(false);
    int btn = resolveKeyName(args[0]);
    if (btn >= 0) return Value::boolean(isMouseDown(btn));
    return Value::boolean(false);
}
Value apiIsMousePressed(const std::vector<Value>& args) {
    if (args.empty()) return Value::boolean(false);
    int btn = resolveKeyName(args[0]);
    if (btn >= 0) return Value::boolean(isMousePressed(btn));
    return Value::boolean(false);
}
Value apiIsMouseReleased(const std::vector<Value>& args) {
    if (args.empty()) return Value::boolean(false);
    int btn = resolveKeyName(args[0]);
    if (btn >= 0) return Value::boolean(isMouseReleased(btn));
    return Value::boolean(false);
}
Value apiGetMouseX(const std::vector<Value>&) {
    return Value::number((double)getMouseX());
}
Value apiGetMouseY(const std::vector<Value>&) {
    return Value::number((double)getMouseY());
}
Value apiBindAction(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::nilVal();
    std::string action = args[0].toString();
    auto [isMouse, code] = resolveBinding(args[1]);
    bindAction(action, isMouse, code);
    return Value::nilVal();
}
Value apiUnbindAction(const std::vector<Value>& args) {
    if (args.empty()) return Value::nilVal();
    std::string action = args[0].toString();
    unbindAction(action);
    return Value::nilVal();
}
Value apiActionDown(const std::vector<Value>& args) {
    if (args.empty()) return Value::boolean(false);
    std::string action = args[0].toString();
    return Value::boolean(isActionDown(action));
}
Value apiActionPressed(const std::vector<Value>& args) {
    if (args.empty()) return Value::boolean(false);
    std::string action = args[0].toString();
    return Value::boolean(isActionPressed(action));
}
Value apiActionReleased(const std::vector<Value>& args) {
    if (args.empty()) return Value::boolean(false);
    std::string action = args[0].toString();
    return Value::boolean(isActionReleased(action));
}
} // namespace yuki
