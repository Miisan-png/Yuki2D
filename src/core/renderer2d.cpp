#include "renderer2d.hpp"
#include <GLFW/glfw3.h>
#include <cmath>
namespace yuki {
Renderer2D::Renderer2D() : spriteCounter(0) {}
void Renderer2D::drawRect(float x, float y, float w, float h, float r, float g, float b) {
    buffer.push_back({RenderCmdType::Rect, x, y, w, h, r, g, b, -1});
}
int Renderer2D::loadSprite(const std::string& path) {
    return ++spriteCounter;
}
void Renderer2D::drawSprite(int id, float x, float y) {
    buffer.push_back({RenderCmdType::Sprite, x, y, 0, 0, 0, 0, 0, id});
}
void Renderer2D::flush(int screenWidth, int screenHeight) {
    if (buffer.empty()) return;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, screenHeight, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    for (const auto& cmd : buffer) {
        if (cmd.type == RenderCmdType::Rect) {
            glColor3f(cmd.r, cmd.g, cmd.b);
            glBegin(GL_QUADS);
            glVertex2f(cmd.x, cmd.y);
            glVertex2f(cmd.x + cmd.w, cmd.y);
            glVertex2f(cmd.x + cmd.w, cmd.y + cmd.h);
            glVertex2f(cmd.x, cmd.y + cmd.h);
            glEnd();
        } else if (cmd.type == RenderCmdType::Sprite) {
            float r = (float)(cmd.id * 123 % 255) / 255.0f;
            float g = (float)(cmd.id * 456 % 255) / 255.0f;
            float b = (float)(cmd.id * 789 % 255) / 255.0f;
            glColor3f(r, g, b);
            glBegin(GL_QUADS);
            glVertex2f(cmd.x, cmd.y);
            glVertex2f(cmd.x + 32, cmd.y);
            glVertex2f(cmd.x + 32, cmd.y + 32);
            glVertex2f(cmd.x, cmd.y + 32);
            glEnd();
        }
    }
    buffer.clear();
}
}