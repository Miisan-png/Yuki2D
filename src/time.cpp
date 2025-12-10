#include "time.hpp"
#include <GLFW/glfw3.h>
namespace yuki {
Time::Time() {
    startTime = glfwGetTime();
    currentTime = startTime;
    previousTime = startTime;
    dt = 0.0f;
}
void Time::update() {
    currentTime = glfwGetTime();
    dt = static_cast<float>(currentTime - previousTime);
    previousTime = currentTime;
}
float Time::deltaTime() const {
    return dt;
}
float Time::elapsed() const {
    return static_cast<float>(currentTime - startTime);
}
}
