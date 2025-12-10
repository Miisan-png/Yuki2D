#pragma once
#include <string>
struct GLFWwindow;
namespace yuki {
class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();
    bool shouldClose() const;
    void pollEvents();
    void swapBuffers();
    void clear(float r, float g, float b, float a);
private:
    GLFWwindow* window;
};
}
