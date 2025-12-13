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

enum class RenderCmdType { Rect, Sprite, Text, SpriteFrame };

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
    struct SpriteFrameData {
        int sheetId = -1;
        int frame = 0;
        SpriteTransform transform;
        float alpha = 1.0f;
    } spriteFrame;
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
    int loadSpriteSheet(const std::string& path, int frameW, int frameH);
    int createSpriteSheetFromFrames(int frameW, int frameH, const std::vector<std::vector<unsigned char>>& frames);
    bool updateSpriteSheetFromFrames(int sheetId, int frameW, int frameH, const std::vector<std::vector<unsigned char>>& frames);
    void drawSpriteFrame(int sheetId, int frame, float x, float y, float rotationDeg, float scaleX, float scaleY, bool flipX, bool flipY, float originX = -1.0f, float originY = -1.0f, float alpha = 1.0f);
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
    struct SpriteSheet {
        unsigned int texture = 0;
        int texW = 0;
        int texH = 0;
        int frameW = 0;
        int frameH = 0;
        int cols = 0;
        int rows = 0;
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

    void setVirtualResolution(int w, int h);
    int getVirtualWidth() const { return virtualW; }
    int getVirtualHeight() const { return virtualH; }

    void cameraSet(float x, float y);
    void cameraSetZoom(float zoom);
    void cameraSetRotation(float deg);
    void cameraFollowTarget(float x, float y);
    void cameraFollowEnable(bool on);
    void cameraFollowLerp(float speed);
    void cameraSetDeadzone(float w, float h);
    void cameraSetPixelSnap(bool on);
    void cameraSetBounds(float x, float y, float w, float h);
    void cameraClearBounds();
    void cameraShake(float intensity, float duration, float frequency = 30.0f);
    void cameraUpdate(double dt);
    float cameraGetX() const { return cameraX; }
    float cameraGetY() const { return cameraY; }
    float cameraGetZoom() const { return cameraZoom; }
    float cameraGetRotation() const { return cameraRotationDeg; }
    bool cameraIsFollowEnabled() const { return cameraFollowOn; }
    bool cameraIsPixelSnapEnabled() const { return cameraPixelSnap; }

    void debugDrawRect(float x, float y, float w, float h, float r, float g, float b);
    void debugDrawLine(float x1, float y1, float x2, float y2, float r, float g, float b);
    void setDebugEnabled(bool enabled);
    bool isDebugEnabled() const { return debugEnabled; }

    void flush(int screenWidth, int screenHeight, bool useCamera = true);

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
    int virtualW = 1280;
    int virtualH = 720;
    float cameraX = 640.0f;
    float cameraY = 360.0f;
    float cameraZoom = 1.0f;
    float cameraRotationDeg = 0.0f;
    bool cameraFollowOn = false;
    float cameraTargetX = 640.0f;
    float cameraTargetY = 360.0f;
    float cameraFollowSpeed = 8.0f;
    bool cameraDeadzoneOn = false;
    float cameraDeadzoneW = 0.0f;
    float cameraDeadzoneH = 0.0f;
    bool cameraPixelSnap = false;
    bool cameraBoundsOn = false;
    float cameraBoundsX = 0.0f;
    float cameraBoundsY = 0.0f;
    float cameraBoundsW = 0.0f;
    float cameraBoundsH = 0.0f;
    float cameraShakeIntensity = 0.0f;
    float cameraShakeDuration = 0.0f;
    float cameraShakeTimeLeft = 0.0f;
    float cameraShakeFrequency = 30.0f;
    float cameraShakeAccum = 0.0f;
    float cameraShakeOffsetX = 0.0f;
    float cameraShakeOffsetY = 0.0f;
    unsigned int shaderProgram = 0;
    unsigned int vbo = 0;
    int attribPos = -1;
    int attribUV = -1;
    int attribColor = -1;
    int attribUseTex = -1;
    int uniformMvp = -1;
    int uniformTex = -1;
    bool graphicsReady = false;
    std::vector<SpriteSheet> spriteSheets;
};

} // namespace yuki
