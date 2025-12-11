#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace yuki {

struct SpriteTransform {
    float x = 0.0f;
    float y = 0.0f;
    float rotationDeg = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    bool flipX = false;
    bool flipY = false;
    float originX = -1.0f; // -1 means use sprite half-width by default
    float originY = -1.0f; // -1 means use sprite half-height by default

    SpriteTransform() = default;
    SpriteTransform(float px, float py, float rotDeg, float sx, float sy, bool fx, bool fy, float ox, float oy)
        : x(px), y(py), rotationDeg(rotDeg), scaleX(sx), scaleY(sy), flipX(fx), flipY(fy), originX(ox), originY(oy) {}
};

enum class RenderCmdType { Rect, Sprite, Text };

struct RenderCmd {
    RenderCmdType type;
    struct RectData {
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
    } rect;
    struct SpriteData {
        int id = -1;
        SpriteTransform transform;
        float alpha = 1.0f;
    } sprite;
    struct TextData {
        int fontId = -1;
        std::string text;
        float x = 0.0f;
        float y = 0.0f;
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
        float a = 1.0f;
        float scale = 1.0f;
        float maxWidth = 0.0f;
        float lineHeight = 0.0f;
        int align = 0; // 0 left, 1 center, 2 right
    } text;
};

class Renderer2D {
public:
    Renderer2D();
    ~Renderer2D();

    void drawRect(float x, float y, float w, float h, float r, float g, float b);
    int loadSprite(const std::string& path);
    void drawSprite(int id, float x, float y);
    void drawSpriteEx(int id, float x, float y, float rotationDeg, float scaleX, float scaleY, bool flipX, bool flipY, float originX = -1.0f, float originY = -1.0f, float alpha = 1.0f);
    int loadFont(const std::string& imagePath, const std::string& metricsPath);
    void drawText(int fontId, const std::string& text, float x, float y);
    void drawTextEx(int fontId, const std::string& text, float x, float y, float scale, float r, float g, float b, float a, const std::string& align, float maxWidth, float lineHeight);
    float measureTextWidth(int fontId, const std::string& text, float scale, float maxWidth, float lineHeight);
    float measureTextHeight(int fontId, const std::string& text, float scale, float maxWidth, float lineHeight);

    struct Texture {
        unsigned int handle;
        int w;
        int h;
    };
    struct FontGlyph {
        float u0, v0, u1, v1;
        int width;
        int advance;
    };
    struct Font {
        unsigned int texture = 0;
        int texW = 0;
        int texH = 0;
        int glyphHeight = 0;
        int lineHeight = 0;
        int spaceAdvance = 4;
        std::unordered_map<int, FontGlyph> glyphs;
    };

    void setCamera(float x, float y);
    void setCameraZoom(float zoom);
    void setCameraRotation(float rotDeg);
    float getCameraRotation() const { return camRot; }
    float getCameraX() const { return camX; }
    float getCameraY() const { return camY; }
    float getCameraZoom() const { return camZoom; }

    void debugDrawRect(float x, float y, float w, float h, float r, float g, float b);
    void debugDrawLine(float x1, float y1, float x2, float y2, float r, float g, float b);
    void setDebugEnabled(bool enabled);
    bool isDebugEnabled() const { return debugEnabled; }

    void flush(int screenWidth, int screenHeight);

private:
    bool initGraphics();
    void destroyGraphics();

    std::vector<RenderCmd> buffer;
    int spriteCounter;
    struct DebugCmd {
        bool isLine;
        float x1, y1, x2, y2, r, g, b;
    };

    std::vector<Texture> textures;
    std::vector<Font> fonts;
    std::vector<DebugCmd> debugBuffer;
    bool debugEnabled;
    float camX;
    float camY;
    float camZoom;
    float camRot;
    unsigned int shaderProgram = 0;
    unsigned int vbo = 0;
    int attribPos = -1;
    int attribUV = -1;
    int attribColor = -1;
    int attribUseTex = -1;
    int uniformMvp = -1;
    int uniformTex = -1;
    bool graphicsReady = false;
};

} // namespace yuki
