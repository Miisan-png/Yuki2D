#include "render_api.hpp"
#include "state.hpp"
#include "value_utils.hpp"
#include "../renderer2d.hpp"
#include "../window.hpp"
#include "../../script/value.hpp"
#include <filesystem>
#include <algorithm>

namespace yuki {
namespace {
BindingsState& st = bindingsState();

std::filesystem::path resolvePath(const std::string& rel) {
    std::filesystem::path p(rel);
    if (p.is_absolute()) return p;
    if (!st.assetBase.empty()) return std::filesystem::path(st.assetBase) / p;
    return p;
}
} // namespace

Value apiSetClearColor(const std::vector<Value>& args) {
    if (args.size() >= 3 && st.window) {
        st.window->setClearColor(args[0].numberVal, args[1].numberVal, args[2].numberVal);
    }
    return Value::nilVal();
}
Value apiDrawRect(const std::vector<Value>& args) {
    if (args.size() >= 7 && st.renderer) {
        st.renderer->drawRect(args[0].numberVal, args[1].numberVal, args[2].numberVal, args[3].numberVal, args[4].numberVal, args[5].numberVal, args[6].numberVal);
    }
    return Value::nilVal();
}
Value apiLoadSprite(const std::vector<Value>& args) {
    if (args.size() < 1 || !st.renderer) return Value::number(-1);
    auto p = resolvePath(args[0].toString());
    return Value::number(st.renderer->loadSprite(p.string()));
}
Value apiLoadSpriteSheet(const std::vector<Value>& args) {
    if (args.size() < 3 || !st.renderer) return Value::number(-1);
    auto p = resolvePath(args[0].toString());
    int fw = (int)args[1].numberVal;
    int fh = (int)args[2].numberVal;
    return Value::number(st.renderer->loadSpriteSheet(p.string(), fw, fh));
}
Value apiLoadFont(const std::vector<Value>& args) {
    if (args.size() < 2 || !st.renderer) return Value::number(-1);
    auto img = resolvePath(args[0].toString());
    auto metrics = resolvePath(args[1].toString());
    return Value::number(st.renderer->loadFont(img.string(), metrics.string()));
}
Value apiDrawSprite(const std::vector<Value>& args) {
    if (args.size() >= 3 && st.renderer) {
        st.renderer->drawSprite((int)args[0].numberVal, args[1].numberVal, args[2].numberVal);
    }
    return Value::nilVal();
}
Value apiDrawSpriteEx(const std::vector<Value>& args) {
    if (args.size() < 5 || !st.renderer) return Value::nilVal();
    int id = (int)args[0].numberVal;
    float x = args[1].numberVal;
    float y = args[2].numberVal;
    float rot = args[3].numberVal;
    float sx = args[4].numberVal;
    float sy = args.size() > 5 ? args[5].numberVal : sx;
    bool fx = args.size() > 6 ? valueToBool(args[6]) : false;
    bool fy = args.size() > 7 ? valueToBool(args[7]) : false;
    float ox = args.size() > 8 ? args[8].numberVal : -1.0f;
    float oy = args.size() > 9 ? args[9].numberVal : -1.0f;
    float alpha = args.size() > 10 ? args[10].numberVal : 1.0f;
    st.renderer->drawSpriteEx(id, x, y, rot, sx, sy, fx, fy, ox, oy, alpha);
    return Value::nilVal();
}
Value apiDrawSpriteFrame(const std::vector<Value>& args) {
    if (args.size() < 9 || !st.renderer) return Value::nilVal();
    int sheetId = (int)args[0].numberVal;
    int frame = (int)args[1].numberVal;
    float x = args[2].numberVal;
    float y = args[3].numberVal;
    float rot = args[4].numberVal;
    float sx = args[5].numberVal;
    float sy = args[6].numberVal;
    bool fx = valueToBool(args[7]);
    bool fy = args.size() > 8 ? valueToBool(args[8]) : false;
    float ox = args.size() > 9 ? args[9].numberVal : -1.0f;
    float oy = args.size() > 10 ? args[10].numberVal : -1.0f;
    float alpha = args.size() > 11 ? args[11].numberVal : 1.0f;
    st.renderer->drawSpriteFrame(sheetId, frame, x, y, rot, sx, sy, fx, fy, ox, oy, alpha);
    return Value::nilVal();
}
Value apiDrawText(const std::vector<Value>& args) {
    if (args.size() < 4 || !st.renderer) return Value::nilVal();
    int fontId = (int)args[0].numberVal;
    std::string text = args[1].toString();
    float x = args[2].numberVal;
    float y = args[3].numberVal;
    float scale = 1.0f;
    float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
    std::string align = "left";
    float maxWidth = 0.0f;
    float lineHeight = 0.0f;
    size_t i = 4;
    bool usedKv = false;
    if (i < args.size() && args[i].isString()) {
        while (i < args.size() && args[i].isString()) {
            std::string key = args[i].toString();
            std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c){ return (char)std::tolower(c); });
            if (key == "scale" && i + 1 < args.size() && args[i + 1].isNumber()) {
                scale = args[i + 1].numberVal;
                i += 2;
                usedKv = true;
                continue;
            }
            if (key == "color" && i + 4 < args.size()) {
                if (args[i + 1].isNumber()) r = args[i + 1].numberVal;
                if (args[i + 2].isNumber()) g = args[i + 2].numberVal;
                if (args[i + 3].isNumber()) b = args[i + 3].numberVal;
                if (args[i + 4].isNumber()) a = args[i + 4].numberVal;
                i += 5;
                usedKv = true;
                continue;
            }
            if (key == "align" && i + 1 < args.size() && args[i + 1].isString()) {
                align = args[i + 1].toString();
                i += 2;
                usedKv = true;
                continue;
            }
            if (key == "max_width" && i + 1 < args.size() && args[i + 1].isNumber()) {
                maxWidth = args[i + 1].numberVal;
                i += 2;
                usedKv = true;
                continue;
            }
            if (key == "line_height" && i + 1 < args.size() && args[i + 1].isNumber()) {
                lineHeight = args[i + 1].numberVal;
                i += 2;
                usedKv = true;
                continue;
            }
            i++;
        }
    }
    if (!usedKv) {
        if (args.size() > 4 && args[4].isNumber()) scale = args[4].numberVal;
        if (args.size() > 5 && args[5].isNumber()) maxWidth = args[5].numberVal;
        if (args.size() > 6 && args[6].isNumber()) lineHeight = args[6].numberVal;
    }
    st.renderer->drawTextEx(fontId, text, x, y, scale, r, g, b, a, align, maxWidth, lineHeight);
    return Value::nilVal();
}
Value apiMeasureTextWidth(const std::vector<Value>& args) {
    if (args.size() < 2 || !st.renderer) return Value::number(0);
    int fontId = (int)args[0].numberVal;
    std::string text = args[1].toString();
    float scale = args.size() > 2 ? args[2].numberVal : 1.0f;
    float maxWidth = args.size() > 3 ? args[3].numberVal : 0.0f;
    float lineHeight = args.size() > 4 ? args[4].numberVal : 0.0f;
    return Value::number(st.renderer->measureTextWidth(fontId, text, scale, maxWidth, lineHeight));
}
Value apiMeasureTextHeight(const std::vector<Value>& args) {
    if (args.size() < 2 || !st.renderer) return Value::number(0);
    int fontId = (int)args[0].numberVal;
    std::string text = args[1].toString();
    float scale = args.size() > 2 ? args[2].numberVal : 1.0f;
    float maxWidth = args.size() > 3 ? args[3].numberVal : 0.0f;
    float lineHeight = args.size() > 4 ? args[4].numberVal : 0.0f;
    return Value::number(st.renderer->measureTextHeight(fontId, text, scale, maxWidth, lineHeight));
}
Value apiSetDebugDrawEnabled(const std::vector<Value>& args) {
    if (args.size() >= 1 && st.renderer) {
        bool on = args[0].isBool() ? args[0].boolVal : (args[0].numberVal != 0);
        st.renderer->setDebugEnabled(on);
    }
    return Value::nilVal();
}
Value apiDebugDrawRect(const std::vector<Value>& args) {
    if (args.size() >= 7 && st.renderer) {
        st.renderer->debugDrawRect(args[0].numberVal, args[1].numberVal, args[2].numberVal, args[3].numberVal, args[4].numberVal, args[5].numberVal, args[6].numberVal);
    }
    return Value::nilVal();
}
Value apiDebugDrawLine(const std::vector<Value>& args) {
    if (args.size() >= 7 && st.renderer) {
        st.renderer->debugDrawLine(args[0].numberVal, args[1].numberVal, args[2].numberVal, args[3].numberVal, args[4].numberVal, args[5].numberVal, args[6].numberVal);
    }
    return Value::nilVal();
}
Value apiSetVirtualResolution(const std::vector<Value>& args) {
    if (args.size() < 2 || !st.renderer) return Value::nilVal();
    int w = (int)args[0].numberVal;
    int h = (int)args[1].numberVal;
    st.renderer->setVirtualResolution(w, h);
    return Value::nilVal();
}

Value apiCameraSet(const std::vector<Value>& args) {
    if (args.size() < 2 || !st.renderer) return Value::nilVal();
    st.renderer->cameraSet((float)args[0].numberVal, (float)args[1].numberVal);
    return Value::nilVal();
}

Value apiCameraSetZoom(const std::vector<Value>& args) {
    if (args.empty() || !st.renderer) return Value::nilVal();
    st.renderer->cameraSetZoom((float)args[0].numberVal);
    return Value::nilVal();
}

Value apiCameraSetRotation(const std::vector<Value>& args) {
    if (args.empty() || !st.renderer) return Value::nilVal();
    st.renderer->cameraSetRotation((float)args[0].numberVal);
    return Value::nilVal();
}

Value apiCameraFollowTarget(const std::vector<Value>& args) {
    if (args.size() < 2 || !st.renderer) return Value::nilVal();
    st.renderer->cameraFollowTarget((float)args[0].numberVal, (float)args[1].numberVal);
    return Value::nilVal();
}

Value apiCameraFollowEnable(const std::vector<Value>& args) {
    if (args.empty() || !st.renderer) return Value::nilVal();
    bool on = args[0].isBool() ? args[0].boolVal : (args[0].isNumber() ? args[0].numberVal != 0.0 : false);
    st.renderer->cameraFollowEnable(on);
    return Value::nilVal();
}

Value apiCameraFollowLerp(const std::vector<Value>& args) {
    if (args.empty() || !st.renderer) return Value::nilVal();
    st.renderer->cameraFollowLerp((float)args[0].numberVal);
    return Value::nilVal();
}
} // namespace yuki
