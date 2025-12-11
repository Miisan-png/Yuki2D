#pragma once
#include <string>
#include <vector>
namespace yuki {
enum class RenderCmdType { Rect, Sprite };
struct RenderCmd {
    RenderCmdType type;
    float x, y, w, h;
    float r, g, b;
    int id;
};
class Renderer2D {
public:
    Renderer2D();
    void drawRect(float x, float y, float w, float h, float r, float g, float b);
    int loadSprite(const std::string& path);
    void drawSprite(int id, float x, float y);
    void flush(int screenWidth, int screenHeight);
private:
    std::vector<RenderCmd> buffer;
    int spriteCounter;
};
}