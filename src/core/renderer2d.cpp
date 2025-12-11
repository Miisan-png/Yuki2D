#include "renderer2d.hpp"
#include <GLFW/glfw3.h>
#include <cmath>
#include "log.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace yuki {

namespace {
    constexpr float kDegToRad = 3.14159265f / 180.0f;

    struct SpriteVerts {
        float pos[4][2];
        float uv[4][2];
    };

    SpriteVerts buildSpriteGeometry(const SpriteTransform& t, float baseW, float baseH) {
        SpriteVerts out{};
        float effectiveScaleX = t.scaleX * (t.flipX ? -1.0f : 1.0f);
        float effectiveScaleY = t.scaleY * (t.flipY ? -1.0f : 1.0f);
        float pivotX = t.originX >= 0.0f ? t.originX : baseW * 0.5f;
        float pivotY = t.originY >= 0.0f ? t.originY : baseH * 0.5f;
        float pivotWorldX = t.x + pivotX * effectiveScaleX;
        float pivotWorldY = t.y + pivotY * effectiveScaleY;
        float rad = t.rotationDeg * kDegToRad;
        float cosr = std::cos(rad);
        float sinr = std::sin(rad);

        float corners[4][2] = {
            {-pivotX * effectiveScaleX,            -pivotY * effectiveScaleY},
            {(baseW - pivotX) * effectiveScaleX,   -pivotY * effectiveScaleY},
            {(baseW - pivotX) * effectiveScaleX,   (baseH - pivotY) * effectiveScaleY},
            {-pivotX * effectiveScaleX,            (baseH - pivotY) * effectiveScaleY}
        };

        for (int i = 0; i < 4; ++i) {
            float rx = corners[i][0];
            float ry = corners[i][1];
            out.pos[i][0] = pivotWorldX + rx * cosr - ry * sinr;
            out.pos[i][1] = pivotWorldY + rx * sinr + ry * cosr;
        }

        float u0 = t.flipX ? 1.0f : 0.0f;
        float u1 = t.flipX ? 0.0f : 1.0f;
        float v0 = t.flipY ? 1.0f : 0.0f;
        float v1 = t.flipY ? 0.0f : 1.0f;

        out.uv[0][0] = u0; out.uv[0][1] = v0;
        out.uv[1][0] = u1; out.uv[1][1] = v0;
        out.uv[2][0] = u1; out.uv[2][1] = v1;
        out.uv[3][0] = u0; out.uv[3][1] = v1;
        return out;
    }
} // namespace

Renderer2D::Renderer2D() : spriteCounter(0), debugEnabled(true), camX(0.0f), camY(0.0f), camZoom(1.0f), camRot(0.0f) {}

Renderer2D::~Renderer2D() {
    for (const auto& tex : textures) {
        if (tex.handle != 0) {
            glDeleteTextures(1, &tex.handle);
        }
    }
}

void Renderer2D::drawRect(float x, float y, float w, float h, float r, float g, float b) {
    RenderCmd cmd{};
    cmd.type = RenderCmdType::Rect;
    cmd.rect = {x, y, w, h, r, g, b};
    buffer.push_back(cmd);
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
    drawSpriteEx(id, x, y, 0.0f, 1.0f, 1.0f, false, false);
}

void Renderer2D::drawSpriteEx(int id, float x, float y, float rotationDeg, float scaleX, float scaleY, bool flipX, bool flipY, float originX, float originY, float alpha) {
    if (id < 0 || id >= (int)textures.size()) return;
    RenderCmd cmd{};
    cmd.type = RenderCmdType::Sprite;
    cmd.sprite.id = id;
    cmd.sprite.transform = SpriteTransform{x, y, rotationDeg, scaleX, scaleY, flipX, flipY, originX, originY};
    cmd.sprite.alpha = alpha;
    buffer.push_back(cmd);
}

void Renderer2D::setCamera(float x, float y) {
    camX = x;
    camY = y;
}

void Renderer2D::setCameraZoom(float zoom) {
    if (zoom <= 0.01f) zoom = 0.01f;
    camZoom = zoom;
}
void Renderer2D::setCameraRotation(float rotDeg) {
    camRot = rotDeg;
}

void Renderer2D::debugDrawRect(float x, float y, float w, float h, float r, float g, float b) {
    debugBuffer.push_back({false, x, y, w, h, r, g, b});
}

void Renderer2D::debugDrawLine(float x1, float y1, float x2, float y2, float r, float g, float b) {
    debugBuffer.push_back({true, x1, y1, x2, y2, r, g, b});
}

void Renderer2D::setDebugEnabled(bool enabled) {
    debugEnabled = enabled;
}

void Renderer2D::flush(int screenWidth, int screenHeight) {
    bool hasRender = !buffer.empty();
    bool hasDebug = debugEnabled && !debugBuffer.empty();
    if (!hasRender && !hasDebug) {
        debugBuffer.clear();
        return;
    }

    glViewport(0, 0, screenWidth, screenHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, screenHeight, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(camZoom, camZoom, 1.0f);
    glTranslatef(-camX, -camY, 0.0f);
    glRotatef(-camRot, 0.0f, 0.0f, 1.0f);

    if (hasRender) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        for (const auto& cmd : buffer) {
            if (cmd.type == RenderCmdType::Rect) {
                glDisable(GL_TEXTURE_2D);
                glColor3f(cmd.rect.r, cmd.rect.g, cmd.rect.b);
                glBegin(GL_QUADS);
                glVertex2f(cmd.rect.x, cmd.rect.y);
                glVertex2f(cmd.rect.x + cmd.rect.w, cmd.rect.y);
                glVertex2f(cmd.rect.x + cmd.rect.w, cmd.rect.y + cmd.rect.h);
                glVertex2f(cmd.rect.x, cmd.rect.y + cmd.rect.h);
                glEnd();
            } else if (cmd.type == RenderCmdType::Sprite) {
                if (cmd.sprite.id < 0 || cmd.sprite.id >= (int)textures.size()) {
                    continue;
                }
                const auto& tex = textures[cmd.sprite.id];
                SpriteVerts verts = buildSpriteGeometry(cmd.sprite.transform, (float)tex.w, (float)tex.h);

                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, tex.handle);
                glColor4f(1.0f, 1.0f, 1.0f, cmd.sprite.alpha);
                glBegin(GL_QUADS);
                for (int i = 0; i < 4; ++i) {
                    glTexCoord2f(verts.uv[i][0], verts.uv[i][1]);
                    glVertex2f(verts.pos[i][0], verts.pos[i][1]);
                }
                glEnd();
                glDisable(GL_TEXTURE_2D);
            }
        }
    }
    buffer.clear();

    if (hasDebug) {
        glDisable(GL_TEXTURE_2D);
        for (const auto& d : debugBuffer) {
            if (d.isLine) {
                glColor3f(d.r, d.g, d.b);
                glBegin(GL_LINES);
                glVertex2f(d.x1, d.y1);
                glVertex2f(d.x2, d.y2);
                glEnd();
            } else {
                glColor3f(d.r, d.g, d.b);
                glBegin(GL_LINE_LOOP);
                glVertex2f(d.x1, d.y1);
                glVertex2f(d.x1 + d.x2, d.y1);
                glVertex2f(d.x1 + d.x2, d.y1 + d.y2);
                glVertex2f(d.x1, d.y1 + d.y2);
                glEnd();
            }
        }
    }
    debugBuffer.clear();
}

} // namespace yuki
