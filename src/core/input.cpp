#include "input.hpp"
#include "window.hpp"
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstring>
namespace yuki {
static GLFWwindow* nativeWindow = nullptr;
static bool currentKeys[GLFW_KEY_LAST + 1];
static bool previousKeys[GLFW_KEY_LAST + 1];
static bool currentMouse[GLFW_MOUSE_BUTTON_LAST + 1];
static bool previousMouse[GLFW_MOUSE_BUTTON_LAST + 1];
static double mouseX = 0.0;
static double mouseY = 0.0;
enum class BindingType { Key, Mouse };
struct ActionBinding {
    BindingType type;
    int code;
};
static std::unordered_map<std::string, std::vector<ActionBinding>> actionBindings;
void initInput(Window& window) {
    nativeWindow = window.getNativeWindow();
    std::memset(currentKeys, 0, sizeof(currentKeys));
    std::memset(previousKeys, 0, sizeof(previousKeys));
    std::memset(currentMouse, 0, sizeof(currentMouse));
    std::memset(previousMouse, 0, sizeof(previousMouse));
    if (nativeWindow) {
        glfwGetCursorPos(nativeWindow, &mouseX, &mouseY);
    }
}
void updateInput(Window& window) {
    if (!nativeWindow) {
        nativeWindow = window.getNativeWindow();
    }
    std::memcpy(previousKeys, currentKeys, sizeof(currentKeys));
    std::memcpy(previousMouse, currentMouse, sizeof(currentMouse));
    for (int i = 0; i <= GLFW_KEY_LAST; ++i) {
        currentKeys[i] = nativeWindow && (glfwGetKey(nativeWindow, i) == GLFW_PRESS);
    }
    for (int i = 0; i <= GLFW_MOUSE_BUTTON_LAST; ++i) {
        currentMouse[i] = nativeWindow && (glfwGetMouseButton(nativeWindow, i) == GLFW_PRESS);
    }
    if (nativeWindow) {
        glfwGetCursorPos(nativeWindow, &mouseX, &mouseY);
    }
}
bool isKeyDown(int key) {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return currentKeys[key];
}
bool isKeyPressed(int key) {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return currentKeys[key] && !previousKeys[key];
}
bool isKeyReleased(int key) {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return !currentKeys[key] && previousKeys[key];
}
bool isMouseDown(int button) {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return currentMouse[button];
}
bool isMousePressed(int button) {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return currentMouse[button] && !previousMouse[button];
}
bool isMouseReleased(int button) {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return !currentMouse[button] && previousMouse[button];
}
double getMouseX() {
    return mouseX;
}
double getMouseY() {
    return mouseY;
}
void bindAction(const std::string& name, bool isMouse, int code) {
    if (code < 0) return;
    ActionBinding b;
    b.type = isMouse ? BindingType::Mouse : BindingType::Key;
    b.code = code;
    actionBindings[name].push_back(b);
}
void unbindAction(const std::string& name) {
    actionBindings.erase(name);
}
static bool checkAction(const std::string& name, bool (*fnKey)(int), bool (*fnMouse)(int)) {
    auto it = actionBindings.find(name);
    if (it == actionBindings.end()) return false;
    for (const auto& b : it->second) {
        if (b.type == BindingType::Key) {
            if (fnKey(b.code)) return true;
        } else {
            if (fnMouse(b.code)) return true;
        }
    }
    return false;
}
bool isActionDown(const std::string& name) {
    return checkAction(name, isKeyDown, isMouseDown);
}
bool isActionPressed(const std::string& name) {
    return checkAction(name, isKeyPressed, isMousePressed);
}
bool isActionReleased(const std::string& name) {
    return checkAction(name, isKeyReleased, isMouseReleased);
}
}
