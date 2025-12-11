#pragma once
#include <string>
namespace yuki {
class Window;
void initInput(Window& window);
void updateInput(Window& window);
bool isKeyDown(int key);
bool isKeyPressed(int key);
bool isKeyReleased(int key);
bool isMouseDown(int button);
bool isMousePressed(int button);
bool isMouseReleased(int button);
double getMouseX();
double getMouseY();
void bindAction(const std::string& name, bool isMouse, int code);
void unbindAction(const std::string& name);
bool isActionDown(const std::string& name);
bool isActionPressed(const std::string& name);
bool isActionReleased(const std::string& name);
}
