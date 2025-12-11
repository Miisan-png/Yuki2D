#include "input.hpp"
#include "window.hpp"
#include <GLFW/glfw3.h>
#include <cstring>
namespace yuki {
static GLFWwindow* nativeWindow = nullptr;
static bool currentKeys[GLFW_KEY_LAST + 1];
static bool previousKeys[GLFW_KEY_LAST + 1];
void initInput(Window& window) {
    nativeWindow = window.getNativeWindow();
    std::memset(currentKeys, 0, sizeof(currentKeys));
    std::memset(previousKeys, 0, sizeof(previousKeys));
}
void updateInput(Window& window) {
    if (!nativeWindow) {
        nativeWindow = window.getNativeWindow();
    }
    std::memcpy(previousKeys, currentKeys, sizeof(currentKeys));
    for (int i = 0; i <= GLFW_KEY_LAST; ++i) {
        currentKeys[i] = nativeWindow && (glfwGetKey(nativeWindow, i) == GLFW_PRESS);
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
}
