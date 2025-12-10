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
    GLFWwindow* getNativeWindow() const;
    int getWidth() const;
    int getHeight() const;
    const std::string& getTitle() const;
private:
    GLFWwindow* window;
    int m_width;
    int m_height;
    std::string m_title;
};
}
