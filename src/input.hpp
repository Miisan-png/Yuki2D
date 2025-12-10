#pragma once
namespace yuki {
class Window;
void initInput(Window& window);
void updateInput();
bool isKeyDown(int key);
bool isKeyPressed(int key);
}
