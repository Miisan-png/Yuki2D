#include "window.hpp"
#include "log.hpp"
#include <GLFW/glfw3.h>
namespace yuki {
Window::Window(int width, int height, const std::string& title) 
    : m_width(width), m_height(height), m_title(title), bgR(0.0f), bgG(0.0f), bgB(0.0f) {
    if (!glfwInit()) {
        logError("Failed to initialize GLFW");
        return;
    }
    // Force a compatibility profile context so fixed-function calls work on platforms like macOS.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        logError("Failed to create window");
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
}
Window::~Window() {
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}
bool Window::shouldClose() const {
    return glfwWindowShouldClose(window);
}
void Window::pollEvents() {
    glfwPollEvents();
}
void Window::swapBuffers() {
    glfwSwapBuffers(window);
}
void Window::clear() {
    glClearColor(bgR, bgG, bgB, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}
void Window::setClearColor(float r, float g, float b) {
    bgR = r;
    bgG = g;
    bgB = b;
}
void Window::getFramebufferSize(int& width, int& height) const {
    if (window) {
        glfwGetFramebufferSize(window, &width, &height);
        return;
    }
    width = m_width;
    height = m_height;
}
GLFWwindow* Window::getNativeWindow() const {
    return window;
}
int Window::getWidth() const {
    int w = m_width;
    int h = m_height;
    getFramebufferSize(w, h);
    return w;
}
int Window::getHeight() const {
    int w = m_width;
    int h = m_height;
    getFramebufferSize(w, h);
    return h;
}
const std::string& Window::getTitle() const {
    return m_title;
}
}
