#pragma once
namespace yuki {
class Window;
void initInput(Window& window);
void updateInput(Window& window);
bool isKeyDown(int key);
bool isKeyPressed(int key);
bool isKeyReleased(int key);
}
