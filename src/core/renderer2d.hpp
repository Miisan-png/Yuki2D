#pragma once
#include <string>
#include <vector>

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

enum class RenderCmdType { Rect, Sprite };

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
};

class Renderer2D {
public:
    Renderer2D();
    ~Renderer2D();

    void drawRect(float x, float y, float w, float h, float r, float g, float b);
    int loadSprite(const std::string& path);
    void drawSprite(int id, float x, float y);
    void drawSpriteEx(int id, float x, float y, float rotationDeg, float scaleX, float scaleY, bool flipX, bool flipY, float originX = -1.0f, float originY = -1.0f, float alpha = 1.0f);

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
    std::vector<RenderCmd> buffer;
    int spriteCounter;
    struct Texture {
        unsigned int handle;
        int w;
        int h;
    };
    struct DebugCmd {
        bool isLine;
        float x1, y1, x2, y2, r, g, b;
    };

    std::vector<Texture> textures;
    std::vector<DebugCmd> debugBuffer;
    bool debugEnabled;
    float camX;
    float camY;
    float camZoom;
    float camRot;
};

} // namespace yuki
