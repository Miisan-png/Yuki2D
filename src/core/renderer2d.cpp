#define GL_GLEXT_PROTOTYPES
#include "renderer2d.hpp"
#include <GLFW/glfw3.h>
#include <cmath>
#include "log.hpp"
#include <vector>
#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace yuki {

namespace {
    constexpr float kDegToRad = 3.14159265f / 180.0f;

    struct Mat4 {
        float m[16];
    };

    Mat4 identity() {
        Mat4 out{};
        std::memset(out.m, 0, sizeof(out.m));
        out.m[0] = out.m[5] = out.m[10] = out.m[15] = 1.0f;
        return out;
    }

    Mat4 ortho(float l, float r, float b, float t, float n, float f) {
        Mat4 m{};
        m.m[0] = 2.0f / (r - l);
        m.m[5] = 2.0f / (t - b);
        m.m[10] = -2.0f / (f - n);
        m.m[12] = -(r + l) / (r - l);
        m.m[13] = -(t + b) / (t - b);
        m.m[14] = -(f + n) / (f - n);
        m.m[15] = 1.0f;
        return m;
    }

    Mat4 mul(const Mat4& a, const Mat4& b) {
        Mat4 out{};
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                out.m[col + row * 4] =
                    a.m[0 + row * 4] * b.m[col + 0] +
                    a.m[1 + row * 4] * b.m[col + 4] +
                    a.m[2 + row * 4] * b.m[col + 8] +
                    a.m[3 + row * 4] * b.m[col + 12];
            }
        }
        return out;
    }

    Mat4 translate(float x, float y, float z) {
        Mat4 m = identity();
        m.m[12] = x;
        m.m[13] = y;
        m.m[14] = z;
        return m;
    }

    Mat4 scale(float sx, float sy, float sz) {
        Mat4 m = identity();
        m.m[0] = sx;
        m.m[5] = sy;
        m.m[10] = sz;
        return m;
    }

    Mat4 rotateZ(float deg) {
        Mat4 m = identity();
        float rad = deg * kDegToRad;
        float c = std::cos(rad);
        float s = std::sin(rad);
        m.m[0] = c;
        m.m[1] = s;
        m.m[4] = -s;
        m.m[5] = c;
        return m;
    }

    unsigned int compileShader(unsigned int type, const char* src) {
        unsigned int shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);
        int success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char info[512];
            glGetShaderInfoLog(shader, 512, nullptr, info);
            logError(std::string("Shader compile failed: ") + info);
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    unsigned int linkProgram(unsigned int vs, unsigned int fs) {
        unsigned int prog = glCreateProgram();
        glAttachShader(prog, vs);
        glAttachShader(prog, fs);
        glBindAttribLocation(prog, 0, "a_pos");
        glBindAttribLocation(prog, 1, "a_uv");
        glBindAttribLocation(prog, 2, "a_color");
        glBindAttribLocation(prog, 3, "a_useTex");
        glLinkProgram(prog);
        int success = 0;
        glGetProgramiv(prog, GL_LINK_STATUS, &success);
        if (!success) {
            char info[512];
            glGetProgramInfoLog(prog, 512, nullptr, info);
            logError(std::string("Program link failed: ") + info);
            glDeleteProgram(prog);
            return 0;
        }
        return prog;
    }

    struct SpriteVerts {
        float pos[4][2];
        float uv[4][2];
    };

    SpriteVerts buildSpriteGeometry(const SpriteTransform& t, float baseW, float baseH) {
        SpriteVerts out{};
        float effectiveScaleX = std::abs(t.scaleX);
        float effectiveScaleY = std::abs(t.scaleY);
        float pivotX = t.originX >= 0.0f ? t.originX : baseW * 0.5f;
        float pivotY = t.originY >= 0.0f ? t.originY : baseH * 0.5f;
        float pivotWorldX = t.x + pivotX * t.scaleX;
        float pivotWorldY = t.y + pivotY * t.scaleY;
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

        float u0 = 0.0f;
        float u1 = 1.0f;
        float v0 = 0.0f;
        float v1 = 1.0f;

        out.uv[0][0] = u0; out.uv[0][1] = v0;
        out.uv[1][0] = u1; out.uv[1][1] = v0;
        out.uv[2][0] = u1; out.uv[2][1] = v1;
        out.uv[3][0] = u0; out.uv[3][1] = v1;
        return out;
    }

    struct Vertex {
        float pos[2];
        float uv[2];
        float color[4];
        float useTex;
    };

    void pushQuad(std::vector<Vertex>& verts, const SpriteVerts& spriteVerts, float r, float g, float b, float a, bool textured) {
        Vertex v0{}, v1{}, v2{}, v3{};
        v0.pos[0] = spriteVerts.pos[0][0]; v0.pos[1] = spriteVerts.pos[0][1];
        v1.pos[0] = spriteVerts.pos[1][0]; v1.pos[1] = spriteVerts.pos[1][1];
        v2.pos[0] = spriteVerts.pos[2][0]; v2.pos[1] = spriteVerts.pos[2][1];
        v3.pos[0] = spriteVerts.pos[3][0]; v3.pos[1] = spriteVerts.pos[3][1];
        v0.uv[0] = spriteVerts.uv[0][0]; v0.uv[1] = spriteVerts.uv[0][1];
        v1.uv[0] = spriteVerts.uv[1][0]; v1.uv[1] = spriteVerts.uv[1][1];
        v2.uv[0] = spriteVerts.uv[2][0]; v2.uv[1] = spriteVerts.uv[2][1];
        v3.uv[0] = spriteVerts.uv[3][0]; v3.uv[1] = spriteVerts.uv[3][1];
        for (int i = 0; i < 4; ++i) {
            v0.color[i] = (i == 3) ? a : (i == 0 ? r : (i == 1 ? g : b));
            v1.color[i] = v0.color[i];
            v2.color[i] = v0.color[i];
            v3.color[i] = v0.color[i];
        }
        v0.useTex = textured ? 1.0f : 0.0f;
        v1.useTex = v0.useTex;
        v2.useTex = v0.useTex;
        v3.useTex = v0.useTex;
        verts.push_back(v0);
        verts.push_back(v1);
        verts.push_back(v2);
        verts.push_back(v0);
        verts.push_back(v2);
        verts.push_back(v3);
    }

    int decodeFirstCodepoint(const std::string& s) {
        if (s.empty()) return -1;
        const unsigned char* p = reinterpret_cast<const unsigned char*>(s.data());
        if (p[0] < 0x80) return p[0];
        if ((p[0] & 0xE0) == 0xC0 && s.size() >= 2) {
            return ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
        }
        if ((p[0] & 0xF0) == 0xE0 && s.size() >= 3) {
            return ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
        }
        if ((p[0] & 0xF8) == 0xF0 && s.size() >= 4) {
            return ((p[0] & 0x07) << 18) | ((p[1] & 0x3F) << 12) | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
        }
        return -1;
    }

    std::string unescapeKey(const std::string& raw) {
        std::string out;
        for (size_t i = 0; i < raw.size(); ++i) {
            if (raw[i] == '\\' && i + 1 < raw.size()) {
                char c = raw[i + 1];
                out.push_back(c);
                i++;
            } else {
                out.push_back(raw[i]);
            }
        }
        return out;
    }

    bool parseBitmapJson(const std::string& path, std::vector<int>& keys, std::unordered_map<int, std::vector<int>>& rows, int& glyphHeight, int& maxWidth) {
        std::ifstream f(path);
        if (!f.is_open()) {
            logError("Failed to open font metrics: " + path);
            return false;
        }
        std::string line;
        glyphHeight = 0;
        maxWidth = 0;
        while (std::getline(f, line)) {
            if (line.find('{') != std::string::npos || line.find('}') != std::string::npos) continue;
            auto q1 = line.find('"');
            if (q1 == std::string::npos) continue;
            auto q2 = line.find('"', q1 + 1);
            if (q2 == std::string::npos) continue;
            std::string keyRaw = line.substr(q1 + 1, q2 - q1 - 1);
            std::string key = unescapeKey(keyRaw);
            auto lb = line.find('[', q2);
            auto rb = line.find(']', q2);
            if (lb == std::string::npos || rb == std::string::npos || rb <= lb) continue;
            std::string nums = line.substr(lb + 1, rb - lb - 1);
            std::vector<int> values;
            std::stringstream ss(nums);
            std::string num;
            while (std::getline(ss, num, ',')) {
                if (num.empty()) continue;
                values.push_back(std::stoi(num));
            }
            if (values.empty()) continue;
            int code = decodeFirstCodepoint(key);
            if (code < 0) continue;
            glyphHeight = (glyphHeight == 0) ? (int)values.size() : glyphHeight;
            rows[code] = values;
            keys.push_back(code);
            int localWidth = 0;
            for (int v : values) {
                for (int bit = 0; bit < 16; ++bit) {
                    if (v & (1 << bit)) localWidth = std::max(localWidth, bit + 1);
                }
            }
            maxWidth = std::max(maxWidth, localWidth);
        }
        std::sort(keys.begin(), keys.end());
        return !keys.empty();
    }

    struct TextLayout {
        std::vector<std::string> lines;
        std::vector<float> widths;
        float totalHeight = 0.0f;
    };

    TextLayout layoutText(const Renderer2D::Font& font, const std::string& text, float scale, float maxWidth, float lineHeightOverride) {
        TextLayout out;
        std::string current;
        float currentWidth = 0.0f;
        size_t i = 0;
        while (i < text.size()) {
            char c = text[i];
            if (c == '\n') {
                out.lines.push_back(current);
                out.widths.push_back(currentWidth * scale);
                current.clear();
                currentWidth = 0.0f;
                i++;
                continue;
            }
            if (c == ' ') {
                int adv = font.spaceAdvance;
                if (maxWidth > 0.0f && currentWidth + adv > maxWidth) {
                    out.lines.push_back(current);
                    out.widths.push_back(currentWidth * scale);
                    current.clear();
                    currentWidth = 0.0f;
                } else {
                    current.push_back(' ');
                    currentWidth += adv;
                }
                i++;
                continue;
            }
            size_t j = i;
            while (j < text.size() && text[j] != ' ' && text[j] != '\n') j++;
            std::string word = text.substr(i, j - i);
            float w = 0.0f;
            for (char wc : word) {
                int code = (unsigned char)wc;
                auto it = font.glyphs.find(code);
                if (it != font.glyphs.end()) w += it->second.advance;
                else w += font.spaceAdvance;
            }
            if (maxWidth > 0.0f && currentWidth > 0.0f && currentWidth + font.spaceAdvance + w > maxWidth) {
                out.lines.push_back(current);
                out.widths.push_back(currentWidth * scale);
                current.clear();
                currentWidth = 0.0f;
            }
            if (!current.empty()) {
                current.push_back(' ');
                currentWidth += font.spaceAdvance;
            }
            current += word;
            currentWidth += w;
            i = j;
        }
        if (!current.empty() || text.empty()) {
            out.lines.push_back(current);
            out.widths.push_back(currentWidth * scale);
        }
        float step = (lineHeightOverride > 0.0f ? lineHeightOverride : (float)font.lineHeight) * scale;
        out.totalHeight = step * (float)out.lines.size();
        return out;
    }
}

Renderer2D::Renderer2D() : spriteCounter(0), debugEnabled(true) {}

Renderer2D::~Renderer2D() {
    for (const auto& tex : textures) {
        if (tex.handle != 0) {
            glDeleteTextures(1, &tex.handle);
        }
    }
    for (const auto& sheet : spriteSheets) {
        if (sheet.texture != 0) {
            glDeleteTextures(1, &sheet.texture);
        }
    }
    for (const auto& f : fonts) {
        if (f.texture != 0) {
            glDeleteTextures(1, &f.texture);
        }
    }
    destroyGraphics();
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

int Renderer2D::loadSpriteSheet(const std::string& path, int frameW, int frameH) {
    if (frameW <= 0 || frameH <= 0) return -1;
    int w, h, channels;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
    if (!data) {
        logError("Failed to load sprite sheet: " + path);
        return -1;
    }
    if (w < frameW || h < frameH) {
        stbi_image_free(data);
        logError("Sprite sheet too small for frame size: " + path);
        return -1;
    }

    unsigned int texId = 0;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    int cols = frameW > 0 ? w / frameW : 0;
    int rows = frameH > 0 ? h / frameH : 0;
    if (cols <= 0 || rows <= 0) {
        glDeleteTextures(1, &texId);
        logError("Invalid frame layout for sheet: " + path);
        return -1;
    }
    SpriteSheet sheet;
    sheet.texture = texId;
    sheet.texW = w;
    sheet.texH = h;
    sheet.frameW = frameW;
    sheet.frameH = frameH;
    sheet.cols = cols;
    sheet.rows = rows;
    spriteSheets.push_back(sheet);
    return (int)spriteSheets.size() - 1;
}

int Renderer2D::createSpriteSheetFromFrames(int frameW, int frameH, const std::vector<std::vector<unsigned char>>& frames) {
    if (frameW <= 0 || frameH <= 0 || frames.empty()) return -1;
    int cols = (int)frames.size();
    int texW = frameW * cols;
    int texH = frameH;
    std::vector<unsigned char> pixels((size_t)texW * (size_t)texH * 4, 0);
    for (size_t i = 0; i < frames.size(); ++i) {
        const auto& f = frames[i];
        if (f.size() < (size_t)frameW * (size_t)frameH * 4) continue;
        int xoff = (int)i * frameW;
        for (int y = 0; y < frameH; ++y) {
            for (int x = 0; x < frameW; ++x) {
                size_t src = ((size_t)y * frameW + x) * 4;
                size_t dst = ((size_t)y * texW + (xoff + x)) * 4;
                pixels[dst + 0] = f[src + 0];
                pixels[dst + 1] = f[src + 1];
                pixels[dst + 2] = f[src + 2];
                pixels[dst + 3] = f[src + 3];
            }
        }
    }
    unsigned int texId = 0;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    SpriteSheet sheet;
    sheet.texture = texId;
    sheet.texW = texW;
    sheet.texH = texH;
    sheet.frameW = frameW;
    sheet.frameH = frameH;
    sheet.cols = cols;
    sheet.rows = 1;
    spriteSheets.push_back(sheet);
    return (int)spriteSheets.size() - 1;
}

bool Renderer2D::updateSpriteSheetFromFrames(int sheetId, int frameW, int frameH, const std::vector<std::vector<unsigned char>>& frames) {
    if (sheetId < 0 || sheetId >= (int)spriteSheets.size()) return false;
    if (frameW <= 0 || frameH <= 0 || frames.empty()) return false;
    int cols = (int)frames.size();
    int texW = frameW * cols;
    int texH = frameH;
    std::vector<unsigned char> pixels((size_t)texW * (size_t)texH * 4, 0);
    for (size_t i = 0; i < frames.size(); ++i) {
        const auto& f = frames[i];
        if (f.size() < (size_t)frameW * (size_t)frameH * 4) continue;
        int xoff = (int)i * frameW;
        for (int y = 0; y < frameH; ++y) {
            for (int x = 0; x < frameW; ++x) {
                size_t src = ((size_t)y * frameW + x) * 4;
                size_t dst = ((size_t)y * texW + (xoff + x)) * 4;
                pixels[dst + 0] = f[src + 0];
                pixels[dst + 1] = f[src + 1];
                pixels[dst + 2] = f[src + 2];
                pixels[dst + 3] = f[src + 3];
            }
        }
    }
    auto& sheet = spriteSheets[sheetId];
    glBindTexture(GL_TEXTURE_2D, sheet.texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    sheet.texW = texW;
    sheet.texH = texH;
    sheet.frameW = frameW;
    sheet.frameH = frameH;
    sheet.cols = cols;
    sheet.rows = 1;
    return true;
}

void Renderer2D::drawSpriteFrame(int sheetId, int frame, float x, float y, float rotationDeg, float scaleX, float scaleY, bool flipX, bool flipY, float originX, float originY, float alpha) {
    if (sheetId < 0 || sheetId >= (int)spriteSheets.size()) return;
    RenderCmd cmd{};
    cmd.type = RenderCmdType::SpriteFrame;
    cmd.spriteFrame.sheetId = sheetId;
    cmd.spriteFrame.frame = frame;
    cmd.spriteFrame.transform = SpriteTransform{x, y, rotationDeg, scaleX, scaleY, flipX, flipY, originX, originY};
    cmd.spriteFrame.alpha = alpha;
    buffer.push_back(cmd);
}

int Renderer2D::loadFont(const std::string& imagePath, const std::string& metricsPath) {
    (void)imagePath;
    std::vector<int> keys;
    std::unordered_map<int, std::vector<int>> rows;
    int glyphHeight = 0;
    int maxWidth = 0;
    if (!parseBitmapJson(metricsPath, keys, rows, glyphHeight, maxWidth)) {
        logError("Failed to parse font metrics: " + metricsPath);
        return -1;
    }
    int cellW = maxWidth + 1;
    int cellH = glyphHeight + 1;
    int cols = (int)std::ceil(std::sqrt((float)keys.size()));
    if (cols < 1) cols = 1;
    int rowsCount = (int)std::ceil(keys.size() / (float)cols);
    int texW = cols * cellW;
    int texH = rowsCount * cellH;
    std::vector<unsigned char> pixels(texW * texH * 4, 0);

    Font font;
    font.texW = texW;
    font.texH = texH;
    font.glyphHeight = glyphHeight;
    font.lineHeight = glyphHeight + 2;

    int idx = 0;
    for (int code : keys) {
        const auto& rowVals = rows[code];
        int localWidth = 0;
        for (int v : rowVals) {
            for (int bit = 0; bit < 16; ++bit) {
                if (v & (1 << bit)) localWidth = std::max(localWidth, bit + 1);
            }
        }
        int gx = (idx % cols) * cellW;
        int gy = (idx / cols) * cellH;
        for (int y = 0; y < glyphHeight; ++y) {
            int row = rowVals[y];
            for (int bit = 0; bit < localWidth; ++bit) {
                if (row & (1 << bit)) {
                    int px = gx + bit;
                    int py = gy + y;
                    int base = (py * texW + px) * 4;
                    pixels[base + 0] = 255;
                    pixels[base + 1] = 255;
                    pixels[base + 2] = 255;
                    pixels[base + 3] = 255;
                }
            }
        }
        FontGlyph g;
        g.u0 = (float)gx / (float)texW;
        g.v0 = (float)gy / (float)texH;
        g.u1 = (float)(gx + localWidth) / (float)texW;
        g.v1 = (float)(gy + glyphHeight) / (float)texH;
        g.width = localWidth;
        g.advance = localWidth + 1;
        font.glyphs[code] = g;
        idx++;
    }

    auto itSpace = font.glyphs.find(' ');
    if (itSpace != font.glyphs.end()) {
        font.spaceAdvance = itSpace->second.advance;
    } else {
        font.spaceAdvance = cellW / 2;
    }

    unsigned int texId = 0;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    font.texture = texId;
    fonts.push_back(font);
    return (int)fonts.size() - 1;
}

void Renderer2D::drawText(int fontId, const std::string& text, float x, float y) {
    drawTextEx(fontId, text, x, y, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, "left", 0.0f, 0.0f);
}

void Renderer2D::drawTextEx(int fontId, const std::string& text, float x, float y, float scale, float r, float g, float b, float a, const std::string& alignStr, float maxWidth, float lineHeight) {
    if (fontId < 0 || fontId >= (int)fonts.size()) return;
    RenderCmd cmd{};
    cmd.type = RenderCmdType::Text;
    cmd.text.fontId = fontId;
    cmd.text.text = text;
    cmd.text.x = x;
    cmd.text.y = y;
    cmd.text.scale = scale;
    cmd.text.r = r;
    cmd.text.g = g;
    cmd.text.b = b;
    cmd.text.a = a;
    cmd.text.maxWidth = maxWidth;
    cmd.text.lineHeight = lineHeight;
    std::string low = alignStr;
    std::transform(low.begin(), low.end(), low.begin(), [](unsigned char c){ return (char)std::tolower(c); });
    if (low == "center") cmd.text.align = 1;
    else if (low == "right") cmd.text.align = 2;
    else cmd.text.align = 0;
    buffer.push_back(cmd);
}

float Renderer2D::measureTextWidth(int fontId, const std::string& text, float scale, float maxWidth, float lineHeight) {
    if (fontId < 0 || fontId >= (int)fonts.size()) return 0.0f;
    Font& font = fonts[fontId];
    TextLayout l = layoutText(font, text, scale, maxWidth, lineHeight);
    float maxw = 0.0f;
    for (float w : l.widths) maxw = std::max(maxw, w);
    return maxw;
}

float Renderer2D::measureTextHeight(int fontId, const std::string& text, float scale, float maxWidth, float lineHeight) {
    if (fontId < 0 || fontId >= (int)fonts.size()) return 0.0f;
    Font& font = fonts[fontId];
    TextLayout l = layoutText(font, text, scale, maxWidth, lineHeight);
    return l.totalHeight;
}

void Renderer2D::setVirtualResolution(int w, int h) {
    if (w > 0 && h > 0) {
        virtualW = w;
        virtualH = h;
    }
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

    if (!graphicsReady) {
        graphicsReady = initGraphics();
        if (!graphicsReady) {
            buffer.clear();
            debugBuffer.clear();
            return;
        }
    }

    glViewport(0, 0, screenWidth, screenHeight);
    Mat4 proj = ortho(0.0f, (float)virtualW, (float)virtualH, 0.0f, -1.0f, 1.0f);

    glUseProgram(shaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(uniformTex, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(attribPos);
    glEnableVertexAttribArray(attribUV);
    glEnableVertexAttribArray(attribColor);
    glEnableVertexAttribArray(attribUseTex);
    glVertexAttribPointer(attribPos, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glVertexAttribPointer(attribUV, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 2));
    glVertexAttribPointer(attribColor, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 4));
    glVertexAttribPointer(attribUseTex, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 8));

    std::vector<Vertex> batch;
    batch.reserve((buffer.size() + debugBuffer.size()) * 6);
    unsigned int currentTex = 0;
    GLenum currentMode = GL_TRIANGLES;
    auto flushBatch = [&](GLenum mode, unsigned int tex) {
        if (batch.empty()) return;
        glBindTexture(GL_TEXTURE_2D, tex);
        glBufferData(GL_ARRAY_BUFFER, batch.size() * sizeof(Vertex), batch.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(mode, 0, (int)batch.size());
        batch.clear();
    };

    // Camera removed: only projection matrix is used
    glUniformMatrix4fv(uniformMvp, 1, GL_FALSE, proj.m);

    if (hasRender) {
        for (const auto& cmd : buffer) {
            if (cmd.type == RenderCmdType::Rect) {
                if (currentMode != GL_TRIANGLES || currentTex != 0) {
                    flushBatch(currentMode, currentTex);
                    currentMode = GL_TRIANGLES;
                    currentTex = 0;
                }
                SpriteVerts verts{};
                verts.pos[0][0] = cmd.rect.x; verts.pos[0][1] = cmd.rect.y;
                verts.pos[1][0] = cmd.rect.x + cmd.rect.w; verts.pos[1][1] = cmd.rect.y;
                verts.pos[2][0] = cmd.rect.x + cmd.rect.w; verts.pos[2][1] = cmd.rect.y + cmd.rect.h;
                verts.pos[3][0] = cmd.rect.x; verts.pos[3][1] = cmd.rect.y + cmd.rect.h;
                verts.uv[0][0] = verts.uv[0][1] = verts.uv[1][0] = verts.uv[1][1] = verts.uv[2][0] = verts.uv[2][1] = verts.uv[3][0] = verts.uv[3][1] = 0.0f;
                pushQuad(batch, verts, cmd.rect.r, cmd.rect.g, cmd.rect.b, 1.0f, false);
            } else if (cmd.type == RenderCmdType::Sprite) {
                if (cmd.sprite.id < 0 || cmd.sprite.id >= (int)textures.size()) {
                    continue;
                }
                const auto& tex = textures[cmd.sprite.id];
                SpriteVerts verts = buildSpriteGeometry(cmd.sprite.transform, (float)tex.w, (float)tex.h);
                if (cmd.sprite.transform.flipX) {
                    for (int i = 0; i < 4; ++i) {
                        verts.uv[i][0] = 1.0f - verts.uv[i][0];
                    }
                }
                if (cmd.sprite.transform.flipY) {
                    for (int i = 0; i < 4; ++i) {
                        verts.uv[i][1] = 1.0f - verts.uv[i][1];
                    }
                }
                if (currentMode != GL_TRIANGLES || currentTex != tex.handle) {
                    flushBatch(currentMode, currentTex);
                    currentMode = GL_TRIANGLES;
                    currentTex = tex.handle;
                }
                pushQuad(batch, verts, 1.0f, 1.0f, 1.0f, cmd.sprite.alpha, true);
            } else if (cmd.type == RenderCmdType::SpriteFrame) {
                if (cmd.spriteFrame.sheetId < 0 || cmd.spriteFrame.sheetId >= (int)spriteSheets.size()) {
                    continue;
                }
                const auto& sheet = spriteSheets[cmd.spriteFrame.sheetId];
                if (sheet.frameW <= 0 || sheet.frameH <= 0 || sheet.texW <= 0 || sheet.texH <= 0) continue;
                int maxFrames = sheet.cols * sheet.rows;
                if (maxFrames <= 0) continue;
                int frameIdx = cmd.spriteFrame.frame % maxFrames;
                if (frameIdx < 0) frameIdx += maxFrames;
                int col = frameIdx % sheet.cols;
                int row = frameIdx / sheet.cols;
                float u0 = (float)(col * sheet.frameW) / (float)sheet.texW;
                float v0 = (float)(row * sheet.frameH) / (float)sheet.texH;
                float u1 = (float)((col + 1) * sheet.frameW) / (float)sheet.texW;
                float v1 = (float)((row + 1) * sheet.frameH) / (float)sheet.texH;
                SpriteVerts verts = buildSpriteGeometry(cmd.spriteFrame.transform, (float)sheet.frameW, (float)sheet.frameH);
                for (int i = 0; i < 4; ++i) {
                    verts.uv[i][0] = verts.uv[i][0] < 0.5f ? u0 : u1;
                    verts.uv[i][1] = verts.uv[i][1] < 0.5f ? v0 : v1;
                }
                if (cmd.spriteFrame.transform.flipX) {
                    for (int i = 0; i < 4; ++i) {
                        verts.uv[i][0] = u0 + (u1 - verts.uv[i][0]);
                    }
                }
                if (cmd.spriteFrame.transform.flipY) {
                    for (int i = 0; i < 4; ++i) {
                        verts.uv[i][1] = v0 + (v1 - verts.uv[i][1]);
                    }
                }
                if (currentMode != GL_TRIANGLES || currentTex != sheet.texture) {
                    flushBatch(currentMode, currentTex);
                    currentMode = GL_TRIANGLES;
                    currentTex = sheet.texture;
                }
                pushQuad(batch, verts, 1.0f, 1.0f, 1.0f, cmd.spriteFrame.alpha, true);
            } else if (cmd.type == RenderCmdType::Text) {
                if (cmd.text.fontId < 0 || cmd.text.fontId >= (int)fonts.size()) continue;
                const Font& font = fonts[cmd.text.fontId];
                TextLayout layout = layoutText(font, cmd.text.text, cmd.text.scale, cmd.text.maxWidth, cmd.text.lineHeight);
                float step = (cmd.text.lineHeight > 0.0f ? cmd.text.lineHeight : (float)font.lineHeight) * cmd.text.scale;
                if (currentMode != GL_TRIANGLES || currentTex != font.texture) {
                    flushBatch(currentMode, currentTex);
                    currentMode = GL_TRIANGLES;
                    currentTex = font.texture;
                }
                for (size_t li = 0; li < layout.lines.size(); ++li) {
                    float lineW = layout.widths[li];
                    float startX = cmd.text.x;
                    if (cmd.text.align == 1) startX -= lineW * 0.5f;
                    else if (cmd.text.align == 2) startX -= lineW;
                    float penX = std::floor(startX + 0.5f);
                    float penY = std::floor(cmd.text.y + step * (float)li + 0.5f);
                    for (char c : layout.lines[li]) {
                        int code = (unsigned char)c;
                        auto itg = font.glyphs.find(code);
                        int adv = font.spaceAdvance;
                        if (itg != font.glyphs.end()) {
                            const FontGlyph& g = itg->second;
                            adv = g.advance;
                            if (g.width > 0) {
                                SpriteVerts verts{};
                                float w = (float)g.width * cmd.text.scale;
                                float h = (float)font.glyphHeight * cmd.text.scale;
                                verts.pos[0][0] = penX; verts.pos[0][1] = penY;
                                verts.pos[1][0] = penX + w; verts.pos[1][1] = penY;
                                verts.pos[2][0] = penX + w; verts.pos[2][1] = penY + h;
                                verts.pos[3][0] = penX; verts.pos[3][1] = penY + h;
                                verts.uv[0][0] = g.u0; verts.uv[0][1] = g.v0;
                                verts.uv[1][0] = g.u1; verts.uv[1][1] = g.v0;
                                verts.uv[2][0] = g.u1; verts.uv[2][1] = g.v1;
                                verts.uv[3][0] = g.u0; verts.uv[3][1] = g.v1;
                                pushQuad(batch, verts, cmd.text.r, cmd.text.g, cmd.text.b, cmd.text.a, true);
                            }
                        }
                        penX += (float)adv * cmd.text.scale;
                    }
                }
            }
        }
    }
    flushBatch(currentMode, currentTex);
    buffer.clear();

    if (hasDebug) {
        for (const auto& d : debugBuffer) {
            if (d.isLine) {
                if (currentMode != GL_LINES || currentTex != 0) {
                    flushBatch(currentMode, currentTex);
                    currentMode = GL_LINES;
                    currentTex = 0;
                }
                Vertex v0{}, v1{};
                v0.pos[0] = d.x1; v0.pos[1] = d.y1;
                v1.pos[0] = d.x2; v1.pos[1] = d.y2;
                v0.uv[0] = v0.uv[1] = v1.uv[0] = v1.uv[1] = 0.0f;
                v0.useTex = v1.useTex = 0.0f;
                v0.color[0] = v1.color[0] = d.r;
                v0.color[1] = v1.color[1] = d.g;
                v0.color[2] = v1.color[2] = d.b;
                v0.color[3] = v1.color[3] = 1.0f;
                batch.push_back(v0);
                batch.push_back(v1);
            } else {
                if (currentMode != GL_LINES || currentTex != 0) {
                    flushBatch(currentMode, currentTex);
                    currentMode = GL_LINES;
                    currentTex = 0;
                }
                float x = d.x1;
                float y = d.y1;
                float w = d.x2;
                float h = d.y2;
                Vertex v0{}, v1{}, v2{}, v3{};
                v0.pos[0] = x; v0.pos[1] = y;
                v1.pos[0] = x + w; v1.pos[1] = y;
                v2.pos[0] = x + w; v2.pos[1] = y + h;
                v3.pos[0] = x; v3.pos[1] = y + h;
                Vertex arr[4] = {v0, v1, v2, v3};
                for (int i = 0; i < 4; ++i) {
                    arr[i].uv[0] = arr[i].uv[1] = 0.0f;
                    arr[i].useTex = 0.0f;
                    arr[i].color[0] = d.r;
                    arr[i].color[1] = d.g;
                    arr[i].color[2] = d.b;
                    arr[i].color[3] = 1.0f;
                }
                batch.push_back(arr[0]); batch.push_back(arr[1]);
                batch.push_back(arr[1]); batch.push_back(arr[2]);
                batch.push_back(arr[2]); batch.push_back(arr[3]);
                batch.push_back(arr[3]); batch.push_back(arr[0]);
            }
        }
    }
    flushBatch(currentMode, currentTex);
    debugBuffer.clear();
    glDisableVertexAttribArray(attribPos);
    glDisableVertexAttribArray(attribUV);
    glDisableVertexAttribArray(attribColor);
    glDisableVertexAttribArray(attribUseTex);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

bool Renderer2D::initGraphics() {
    const char* vsSrc =
        "#version 120\n"
        "attribute vec2 a_pos;\n"
        "attribute vec2 a_uv;\n"
        "attribute vec4 a_color;\n"
        "attribute float a_useTex;\n"
        "uniform mat4 u_mvp;\n"
        "varying vec2 v_uv;\n"
        "varying vec4 v_color;\n"
        "varying float v_useTex;\n"
        "void main() {\n"
        " v_uv = a_uv;\n"
        " v_color = a_color;\n"
        " v_useTex = a_useTex;\n"
        " gl_Position = u_mvp * vec4(a_pos, 0.0, 1.0);\n"
        "}\n";
    const char* fsSrc =
        "#version 120\n"
        "uniform sampler2D u_tex;\n"
        "varying vec2 v_uv;\n"
        "varying vec4 v_color;\n"
        "varying float v_useTex;\n"
        "void main() {\n"
        " vec4 c = v_color;\n"
        " if (v_useTex > 0.5) {\n"
        "  c *= texture2D(u_tex, v_uv);\n"
        " }\n"
        " gl_FragColor = c;\n"
        "}\n";
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vsSrc);
    if (!vs) return false;
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fsSrc);
    if (!fs) {
        glDeleteShader(vs);
        return false;
    }
    shaderProgram = linkProgram(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!shaderProgram) return false;
    attribPos = 0;
    attribUV = 1;
    attribColor = 2;
    attribUseTex = 3;
    uniformMvp = glGetUniformLocation(shaderProgram, "u_mvp");
    uniformTex = glGetUniformLocation(shaderProgram, "u_tex");
    glGenBuffers(1, &vbo);
    return uniformMvp >= 0 && uniformTex >= 0 && vbo != 0;
}

void Renderer2D::destroyGraphics() {
    if (vbo != 0) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    if (shaderProgram != 0) {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }
    graphicsReady = false;
}

} // namespace yuki
