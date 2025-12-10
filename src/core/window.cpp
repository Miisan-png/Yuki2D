#include "window.hpp"
#include <GLFW/glfw3.h>
#include <cstdlib>
namespace yuki {
Window::Window(int width, int height, const std::string& title)
    : m_width(width), m_height(height), m_title(title) {
    if (!glfwInit()) {
        std::exit(EXIT_FAILURE);
    }
    window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
}
Window::~Window() {
    glfwDestroyWindow(window);
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
void Window::clear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}
GLFWwindow* Window::getNativeWindow() const {
    return window;
}
int Window::getWidth() const {
    return m_width;
}
int Window::getHeight() const {
    return m_height;
}
const std::string& Window::getTitle() const {
    return m_title;
}
}
