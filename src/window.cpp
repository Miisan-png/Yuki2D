#include "window.hpp"
#include <GLFW/glfw3.h>
#include <cstdlib>
namespace yuki {
Window::Window(int width, int height, const std::string& title) {
    if (!glfwInit()) {
        std::exit(EXIT_FAILURE);
    }
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
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
}
