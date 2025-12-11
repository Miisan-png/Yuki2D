#include "renderer2d.hpp"
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <iostream>
#include "log.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
namespace yuki {
Renderer2D::Renderer2D() : spriteCounter(0) {}
Renderer2D::~Renderer2D() {
    for (const auto& tex : textures) {
        if (tex.handle != 0) {
            glDeleteTextures(1, &tex.handle);
        }
    }
}
void Renderer2D::drawRect(float x, float y, float w, float h, float r, float g, float b) {
    buffer.push_back({RenderCmdType::Rect, x, y, w, h, r, g, b, -1});
}
int Renderer2D::loadSprite(const std::string& path) {
    int w, h, channels;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
    if (!data) {
        logError("Failed to load sprite: " + path);
        return -1;
    }

    unsigned int texId = 0;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    textures.push_back({texId, w, h});
    spriteCounter++;
    return spriteCounter - 1;
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
            if (cmd.id >= 0 && cmd.id < (int)textures.size()) {
                const auto& tex = textures[cmd.id];
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, tex.handle);
                glColor3f(1.0f, 1.0f, 1.0f);
                glBegin(GL_QUADS);
                glTexCoord2f(0.0f, 0.0f); glVertex2f(cmd.x, cmd.y);
                glTexCoord2f(1.0f, 0.0f); glVertex2f(cmd.x + tex.w, cmd.y);
                glTexCoord2f(1.0f, 1.0f); glVertex2f(cmd.x + tex.w, cmd.y + tex.h);
                glTexCoord2f(0.0f, 1.0f); glVertex2f(cmd.x, cmd.y + tex.h);
                glEnd();
                glDisable(GL_TEXTURE_2D);
            }
        }
    }
    buffer.clear();
}
}
