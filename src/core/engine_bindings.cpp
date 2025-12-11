#include "engine_bindings.hpp"
#include "window.hpp"
#include "renderer2d.hpp"
#include "log.hpp"
#include "../script/value.hpp"
#include "../script/interpreter.hpp"
#include "../script/yuki_script_loader.hpp"
#include "../script/token.hpp"
#include "../script/parser.hpp"
#include "../script/ast.hpp"
#include "input.hpp"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <filesystem>
#include <utility>
#include <vector>
#include <set>
namespace yuki {
static Window* g_Window = nullptr;
static Renderer2D* g_Renderer = nullptr;
static Interpreter* g_Interpreter = nullptr;
static std::string g_AssetBase;
static std::vector<std::filesystem::path> g_ImportPaths;
struct Area {
    float x, y, w, h;
    std::string tag;
};
static std::vector<Area> g_Areas;
static std::unordered_map<std::string, bool> g_AreaState;
struct SpriteState {
    float x = 0.0f;
    float y = 0.0f;
    float rotation = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float alpha = 1.0f;
    bool overrideX = false;
    bool overrideY = false;
    bool overrideRot = false;
    bool overrideScaleX = false;
    bool overrideScaleY = false;
    bool overrideAlpha = false;
};
static std::unordered_map<int, SpriteState> g_SpriteStates;
struct Animation {
    int id = -1;
    int sheetId = -1;
    std::vector<int> frames;
    double fps = 0.0;
    bool loop = false;
    double accumulator = 0.0;
    int currentIndex = 0;
    bool playing = false;
    SpriteTransform transform;
    float alpha = 1.0f;
};
static std::unordered_map<int, Animation> g_Animations;
static int g_AnimationCounter = 1;
struct Collider {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
    std::string tag;
    bool solid = true;
};
static std::vector<Collider> g_Colliders;
enum class TweenTargetType { None, Sprite };
enum class TweenType { Value, Property };
struct TweenTarget {
    TweenTargetType type = TweenTargetType::None;
    int id = -1;
};
struct Tween {
    int id = -1;
    TweenType type = TweenType::Value;
    double from = 0.0;
    double to = 0.0;
    double duration = 0.0;
    double elapsed = 0.0;
    bool active = false;
    bool paused = false;
    bool canceled = false;
    std::string easing = "linear";
    double current = 0.0;
    TweenTarget target;
    std::string property;
    Value onComplete = Value::nilVal();
};
struct Sequence {
    int id = -1;
    std::vector<int> tweens;
    int currentIndex = 0;
    bool playing = false;
    bool finished = false;
};
struct ParallelGroup {
    int id = -1;
    std::vector<int> tweens;
    bool playing = false;
    bool finished = false;
};
static std::unordered_map<int, Tween> g_Tweens;
static std::unordered_map<int, Sequence> g_Sequences;
static std::unordered_map<int, ParallelGroup> g_Parallels;
static std::unordered_map<int, int> g_TweenSequenceOwner;
static std::unordered_map<int, int> g_TweenParallelOwner;
static int g_TweenCounter = 1;
static int g_SequenceCounter = 1;
static int g_ParallelCounter = 1;
static std::unordered_set<std::string> g_LoadedModules;
void EngineBindings::init(Window* window, Renderer2D* renderer, Interpreter* interpreter) {
    g_Window = window;
    g_Renderer = renderer;
    g_Interpreter = interpreter;
}
void EngineBindings::setAssetBase(const std::string& base) {
    g_AssetBase = base;
    if (!base.empty()) {
        std::filesystem::path p(base);
        g_ImportPaths.push_back(p);
    }
}

static std::unordered_map<std::string, int> buildKeyMap() {
    std::unordered_map<std::string, int> m;
    m["left"] = GLFW_KEY_LEFT;
    m["right"] = GLFW_KEY_RIGHT;
    m["up"] = GLFW_KEY_UP;
    m["down"] = GLFW_KEY_DOWN;
    m["space"] = GLFW_KEY_SPACE;
    m["enter"] = GLFW_KEY_ENTER;
    m["esc"] = GLFW_KEY_ESCAPE;
    m["escape"] = GLFW_KEY_ESCAPE;
    m["shift"] = GLFW_KEY_LEFT_SHIFT;
    m["ctrl"] = GLFW_KEY_LEFT_CONTROL;
    m["alt"] = GLFW_KEY_LEFT_ALT;
    for (char c = 'a'; c <= 'z'; ++c) {
        std::string s(1, c);
        m[s] = GLFW_KEY_A + (c - 'a');
    }
    for (char c = '0'; c <= '9'; ++c) {
        std::string s(1, c);
        m[s] = GLFW_KEY_0 + (c - '0');
    }
    return m;
}

static const std::unordered_map<std::string, int> kKeyMap = buildKeyMap();
static std::unordered_map<std::string, int> buildMouseMap() {
    std::unordered_map<std::string, int> m;
    m["mouse_left"] = GLFW_MOUSE_BUTTON_LEFT;
    m["mouse_right"] = GLFW_MOUSE_BUTTON_RIGHT;
    m["mouse_middle"] = GLFW_MOUSE_BUTTON_MIDDLE;
    m["mouse_4"] = GLFW_MOUSE_BUTTON_4;
    m["mouse_5"] = GLFW_MOUSE_BUTTON_5;
    m["mouse_6"] = GLFW_MOUSE_BUTTON_6;
    m["mouse_7"] = GLFW_MOUSE_BUTTON_7;
    m["mouse_8"] = GLFW_MOUSE_BUTTON_8;
    return m;
}
static const std::unordered_map<std::string, int> kMouseMap = buildMouseMap();

static int resolveKey(const Value& keyVal) {
    if (keyVal.isString()) {
        std::string name = keyVal.stringVal;
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return (char)std::tolower(c); });
        auto it = kKeyMap.find(name);
        if (it != kKeyMap.end()) return it->second;
        logError("Unknown key name '" + name + "'");
        return -1;
    }
    if (keyVal.isNumber()) {
        return (int)keyVal.numberVal;
    }
    logError("Unknown key type for input query");
    return -1;
}
static std::pair<bool, int> resolveActionBinding(const Value& v) {
    if (v.isString()) {
        std::string name = v.stringVal;
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return (char)std::tolower(c); });
        auto keyIt = kKeyMap.find(name);
        if (keyIt != kKeyMap.end()) return {false, keyIt->second};
        auto mouseIt = kMouseMap.find(name);
        if (mouseIt != kMouseMap.end()) return {true, mouseIt->second};
        logError("Unknown input '" + name + "'");
        return {false, -1};
    }
    if (v.isNumber()) {
        return {false, (int)v.numberVal};
    }
    logError("Unknown input type for action binding");
    return {false, -1};
}
static int resolveMouseButton(const Value& val) {
    if (val.isString()) {
        std::string name = val.stringVal;
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return (char)std::tolower(c); });
        auto it = kMouseMap.find(name);
        if (it != kMouseMap.end()) return it->second;
        logError("Unknown mouse button '" + name + "'");
        return -1;
    }
    if (val.isNumber()) {
        return (int)val.numberVal;
    }
    logError("Unknown mouse button type");
    return -1;
}
static void applySpriteOverrides(int id, float& x, float& y, float& rot, float& sx, float& sy, float& alpha) {
    auto& s = g_SpriteStates[id];
    if (s.overrideX) x = s.x; else s.x = x;
    if (s.overrideY) y = s.y; else s.y = y;
    if (s.overrideRot) rot = s.rotation; else s.rotation = rot;
    if (s.overrideScaleX) sx = s.scaleX; else s.scaleX = sx;
    if (s.overrideScaleY) sy = s.scaleY; else s.scaleY = sy;
    if (s.overrideAlpha) alpha = s.alpha; else s.alpha = alpha;
}
static std::vector<int> toFrameList(const Value& v) {
    std::vector<int> out;
    if (!v.isArray() || !v.arrayPtr) return out;
    for (const auto& item : *v.arrayPtr) {
        if (item.isNumber()) out.push_back((int)item.numberVal);
    }
    return out;
}
static Animation* getAnimation(int id) {
    auto it = g_Animations.find(id);
    if (it == g_Animations.end()) return nullptr;
    return &it->second;
}
static bool rectsOverlap(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh) {
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}
static double clamp01(double v) {
    if (v < 0.0) return 0.0;
    if (v > 1.0) return 1.0;
    return v;
}
static double applyEasing(double t, const std::string& easing) {
    double u = clamp01(t);
    if (easing == "ease_in") return u * u;
    if (easing == "ease_out") return u * (2.0 - u);
    if (easing == "ease_in_out") {
        if (u < 0.5) return 2.0 * u * u;
        double k = -2.0 * u + 2.0;
        return 1.0 - (k * k) / 2.0;
    }
    return u;
}
static bool getPropertyValue(const TweenTarget& target, const std::string& prop, float& out) {
    if (!g_Renderer) return false;
    if (target.type == TweenTargetType::Sprite) {
        auto it = g_SpriteStates.find(target.id);
        if (it == g_SpriteStates.end()) return false;
        const auto& s = it->second;
        if (prop == "x") { out = s.x; return true; }
        if (prop == "y") { out = s.y; return true; }
        if (prop == "rotation") { out = s.rotation; return true; }
        if (prop == "scale_x") { out = s.scaleX; return true; }
        if (prop == "scale_y") { out = s.scaleY; return true; }
        if (prop == "alpha") { out = s.alpha; return true; }
        return false;
    }
    return false;
}
static void setPropertyValue(const TweenTarget& target, const std::string& prop, float v) {
    if (!g_Renderer) return;
    if (target.type == TweenTargetType::Sprite) {
        auto& s = g_SpriteStates[target.id];
        if (prop == "x") { s.x = v; s.overrideX = true; }
        else if (prop == "y") { s.y = v; s.overrideY = true; }
        else if (prop == "rotation") { s.rotation = v; s.overrideRot = true; }
        else if (prop == "scale_x") { s.scaleX = v; s.overrideScaleX = true; }
        else if (prop == "scale_y") { s.scaleY = v; s.overrideScaleY = true; }
        else if (prop == "alpha") { s.alpha = v; s.overrideAlpha = true; }
    }
}
static TweenTarget parseTarget(const Value& obj) {
    TweenTarget tgt;
    if (obj.isNumber()) {
        tgt.type = TweenTargetType::Sprite;
        tgt.id = (int)obj.numberVal;
    }
    return tgt;
}
static int nextTweenId() { return g_TweenCounter++; }
static int nextSequenceId() { return g_SequenceCounter++; }
static int nextParallelId() { return g_ParallelCounter++; }
static void resetTween(Tween& t, bool pause, bool refreshProperty) {
    if (refreshProperty && t.type == TweenType::Property) {
        float cur = (float)t.from;
        if (getPropertyValue(t.target, t.property, cur)) t.from = cur;
    }
    t.elapsed = 0.0;
    t.current = t.from;
    t.active = true;
    t.canceled = false;
    t.paused = pause;
}
static void finishTween(Tween& t) {
    t.active = false;
    t.paused = false;
}
static void invokeTweenCallback(Tween& t) {
    if (!g_Interpreter) return;
    if (!t.onComplete.isFunction()) return;
    std::vector<Value> args;
    g_Interpreter->callFunction(t.onComplete, args);
}
static int createValueTween(double from, double to, double duration, const std::string& easing) {
    int id = nextTweenId();
    Tween t;
    t.id = id;
    t.type = TweenType::Value;
    t.from = from;
    t.to = to;
    t.duration = duration;
    t.easing = easing;
    resetTween(t, false, false);
    g_Tweens[id] = t;
    return id;
}
static int createPropertyTween(const TweenTarget& target, const std::string& prop, double to, double duration, const std::string& easing) {
    float start = 0.0f;
    getPropertyValue(target, prop, start);
    int id = nextTweenId();
    Tween t;
    t.id = id;
    t.type = TweenType::Property;
    t.from = start;
    t.to = to;
    t.duration = duration;
    t.easing = easing;
    t.target = target;
    t.property = prop;
    resetTween(t, false, false);
    g_Tweens[id] = t;
    return id;
}
static void updateTween(Tween& t, double dt) {
    if (!t.active || t.paused || t.canceled) return;
    if (t.duration <= 0.0) {
        t.current = t.to;
        if (t.type == TweenType::Property) setPropertyValue(t.target, t.property, (float)t.current);
        finishTween(t);
        invokeTweenCallback(t);
        return;
    }
    t.elapsed += dt;
    double progress = clamp01(t.elapsed / t.duration);
    double eased = applyEasing(progress, t.easing);
    t.current = t.from + (t.to - t.from) * eased;
    if (t.type == TweenType::Property) {
        setPropertyValue(t.target, t.property, (float)t.current);
    }
    if (progress >= 1.0) {
        finishTween(t);
        invokeTweenCallback(t);
    }
}
static void updateSequences(double dt) {
    for (auto& kv : g_Sequences) {
        auto& seq = kv.second;
        if (!seq.playing || seq.finished) continue;
        while (seq.currentIndex < (int)seq.tweens.size()) {
            int tid = seq.tweens[seq.currentIndex];
            auto it = g_Tweens.find(tid);
            if (it == g_Tweens.end()) {
                seq.currentIndex++;
                continue;
            }
            auto& tw = it->second;
            if (tw.elapsed == 0.0) resetTween(tw, false, true);
            tw.paused = false;
            updateTween(tw, dt);
            if (!tw.active) {
                seq.currentIndex++;
                continue;
            }
            break;
        }
        if (seq.currentIndex >= (int)seq.tweens.size()) {
            seq.finished = true;
            seq.playing = false;
        }
    }
}
static void updateParallels(double dt) {
    for (auto& kv : g_Parallels) {
        auto& grp = kv.second;
        if (!grp.playing || grp.finished) continue;
        bool allDone = true;
        for (int tid : grp.tweens) {
            auto it = g_Tweens.find(tid);
            if (it == g_Tweens.end()) continue;
            auto& tw = it->second;
            if (tw.elapsed == 0.0) resetTween(tw, false, true);
            tw.paused = false;
            updateTween(tw, dt);
            if (tw.active) allDone = false;
        }
        if (allDone) {
            grp.finished = true;
            grp.playing = false;
        }
    }
}
static void updateStandaloneTweens(double dt) {
    for (auto& kv : g_Tweens) {
        auto& t = kv.second;
        if (!t.active) continue;
        if (t.paused) continue;
        if (t.canceled) continue;
        auto seqIt = g_TweenSequenceOwner.find(t.id);
        if (seqIt != g_TweenSequenceOwner.end()) {
            auto s = g_Sequences.find(seqIt->second);
            if (s != g_Sequences.end() && s->second.playing && !s->second.finished) continue;
        }
        auto parIt = g_TweenParallelOwner.find(t.id);
        if (parIt != g_TweenParallelOwner.end()) {
            auto p = g_Parallels.find(parIt->second);
            if (p != g_Parallels.end() && p->second.playing && !p->second.finished) continue;
        }
        updateTween(t, dt);
    }
}
static void cleanupFinishedTweens() {
    std::vector<int> toRemove;
    toRemove.reserve(g_Tweens.size());
    for (const auto& kv : g_Tweens) {
        const auto& t = kv.second;
        if (!t.active && !t.paused) {
            toRemove.push_back(kv.first);
        }
    }
    for (int id : toRemove) {
        g_TweenSequenceOwner.erase(id);
        g_TweenParallelOwner.erase(id);
        g_Tweens.erase(id);
    }
}
static void cleanupFinishedGroups() {
    std::vector<int> seqRemove;
    seqRemove.reserve(g_Sequences.size());
    for (const auto& kv : g_Sequences) {
        if (kv.second.finished) {
            seqRemove.push_back(kv.first);
        }
    }
    for (int id : seqRemove) {
        g_Sequences.erase(id);
    }

    std::vector<int> parRemove;
    parRemove.reserve(g_Parallels.size());
    for (const auto& kv : g_Parallels) {
        if (kv.second.finished) {
            parRemove.push_back(kv.first);
        }
    }
    for (int id : parRemove) {
        g_Parallels.erase(id);
    }
}
static void updateAnimations(double dt) {
    if (g_Animations.empty()) return;
    for (auto& kv : g_Animations) {
        Animation& a = kv.second;
        if (!a.playing) continue;
        if (a.frames.empty()) continue;
        if (a.fps <= 0.0) continue;
        double step = 1.0 / a.fps;
        a.accumulator += dt;
        while (a.accumulator >= step) {
            a.accumulator -= step;
            a.currentIndex++;
            if (a.currentIndex >= (int)a.frames.size()) {
                if (a.loop) {
                    a.currentIndex = 0;
                } else {
                    a.currentIndex = (int)a.frames.size() - 1;
                    a.playing = false;
                    break;
                }
            }
        }
    }
}
static void debugDrawColliders() {
    if (!g_Renderer) return;
    if (!g_Renderer->isDebugEnabled()) return;
    if (g_Colliders.empty()) return;
    for (size_t i = 0; i < g_Colliders.size(); ++i) {
        const auto& c = g_Colliders[i];
        if (c.w <= 0.0f || c.h <= 0.0f) continue;
        std::hash<std::string> h;
        size_t hv = h(c.tag);
        float r = ((hv >> 0) & 0xFF) / 255.0f;
        float g = ((hv >> 8) & 0xFF) / 255.0f;
        float b = ((hv >> 16) & 0xFF) / 255.0f;
        g_Renderer->debugDrawRect(c.x, c.y, c.w, c.h, r, g, b);
    }
}
void EngineBindings::update(double dt) {
    updateSequences(dt);
    updateParallels(dt);
    updateStandaloneTweens(dt);
    cleanupFinishedTweens();
    cleanupFinishedGroups();
    updateAnimations(dt);
    debugDrawColliders();
}
Value engineLog(const std::vector<Value>& args) {
    std::string msg;
    for (size_t i = 0; i < args.size(); i++) {
        msg += args[i].toString();
    }
    printf("[INFO] %s\n", msg.c_str());
    return Value::nilVal();
}
Value engineImport(const std::vector<Value>& args) {
    if (args.empty() || !g_Interpreter) return Value::nilVal();
    std::filesystem::path p(args[0].toString());
    if (p.extension().empty()) p.replace_extension(".ys");
    std::vector<std::filesystem::path> candidates;
    if (p.is_absolute()) candidates.push_back(p);
    else {
        for (const auto& base : g_ImportPaths) candidates.push_back(base / p);
        candidates.push_back(p);
        std::filesystem::path modulesDir = std::filesystem::path("scripts/modules") / p;
        candidates.push_back(modulesDir);
    }
    std::filesystem::path found;
    for (const auto& cand : candidates) {
        if (std::filesystem::exists(cand)) { found = cand; break; }
    }
    if (found.empty()) {
        logError("Import path not found: " + p.string());
        return Value::nilVal();
    }
    std::error_code ec;
    std::filesystem::path canon = std::filesystem::weakly_canonical(found, ec);
    if (ec) canon = found;
    std::string key = canon.string();
    std::optional<std::string> alias;
    if (args.size() > 1 && args[1].isString()) alias = args[1].toString();
    if (g_LoadedModules.count(key)) {
        if (alias) {
            auto envVal = g_Interpreter->globals->get(*alias);
            if (envVal.has_value()) return envVal.value();
        }
        auto exports = g_Interpreter->globals->get("exports");
        if (exports.has_value()) return exports.value();
        return Value::nilVal();
    }
    logInfo("Importing " + p.string());
    ScriptLoader loader(canon.string());
    std::string content = loader.load();
    Tokenizer tokenizer(content);
    std::vector<Token> tokens = tokenizer.scanTokens();
    Parser parser(tokens);
    std::vector<std::unique_ptr<Stmt>> statements = parser.parse();
    if (parser.hadError()) {
        for (const auto& err : parser.getErrors()) {
            logError(err);
        }
        return Value::nilVal();
    }
    logInfo("Executing module " + p.string());
    g_Interpreter->exec(statements);
    if (g_Interpreter->hasRuntimeErrors()) {
        for (const auto& err : g_Interpreter->getRuntimeErrors()) {
            logError(err);
        }
        return Value::nilVal();
    }
    g_Interpreter->retainModule(std::move(statements));
    g_LoadedModules.insert(key);
    if (alias) {
        auto envVal = g_Interpreter->globals->get(*alias);
        if (envVal.has_value()) return envVal.value();
    }
    auto exports = g_Interpreter->globals->get("exports");
    if (exports.has_value()) return exports.value();
    return Value::nilVal();
}
Value engineTime(const std::vector<Value>&) {
    return Value::number(glfwGetTime());
}
Value engineRandom(const std::vector<Value>& args) {
    if (args.empty() || args[0].type != ValueType::Number) return Value::number(0);
    double max = args[0].numberVal;
    double r = (double(rand()) / double(RAND_MAX)) * max;
    return Value::number(r);
}
Value engineSetClearColor(const std::vector<Value>& args) {
    if (args.size() >= 3 && g_Window) {
        g_Window->setClearColor(args[0].numberVal, args[1].numberVal, args[2].numberVal);
    }
    return Value::nilVal();
}
Value engineDrawRect(const std::vector<Value>& args) {
    if (args.size() >= 7 && g_Renderer) {
        g_Renderer->drawRect(args[0].numberVal, args[1].numberVal, args[2].numberVal, args[3].numberVal, args[4].numberVal, args[5].numberVal, args[6].numberVal);
    }
    return Value::nilVal();
}
Value engineLoadSprite(const std::vector<Value>& args) {
    if (args.size() >= 1 && g_Renderer) {
        std::filesystem::path p(args[0].stringVal);
        if (!p.is_absolute() && !g_AssetBase.empty()) {
            p = std::filesystem::path(g_AssetBase) / p;
        }
        return Value::number(g_Renderer->loadSprite(p.string()));
    }
    return Value::number(-1);
}
Value engineLoadSpriteSheet(const std::vector<Value>& args) {
    if (args.size() < 3 || !g_Renderer) return Value::number(-1);
    std::filesystem::path p(args[0].toString());
    if (!p.is_absolute() && !g_AssetBase.empty()) p = std::filesystem::path(g_AssetBase) / p;
    int fw = (int)args[1].numberVal;
    int fh = (int)args[2].numberVal;
    return Value::number(g_Renderer->loadSpriteSheet(p.string(), fw, fh));
}
Value engineLoadFont(const std::vector<Value>& args) {
    if (args.size() < 2 || !g_Renderer) return Value::number(-1);
    std::filesystem::path img(args[0].toString());
    std::filesystem::path metrics(args[1].toString());
    if (!img.is_absolute() && !g_AssetBase.empty()) img = std::filesystem::path(g_AssetBase) / img;
    if (!metrics.is_absolute() && !g_AssetBase.empty()) metrics = std::filesystem::path(g_AssetBase) / metrics;
    return Value::number(g_Renderer->loadFont(img.string(), metrics.string()));
}
Value engineDrawSprite(const std::vector<Value>& args) {
    if (args.size() >= 3 && g_Renderer) {
        int id = (int)args[0].numberVal;
        float x = args[1].numberVal;
        float y = args[2].numberVal;
        float rot = 0.0f;
        float sx = 1.0f;
        float sy = 1.0f;
        float alpha = 1.0f;
        applySpriteOverrides(id, x, y, rot, sx, sy, alpha);
        g_Renderer->drawSpriteEx(id, x, y, rot, sx, sy, false, false, -1.0f, -1.0f, alpha);
    }
    return Value::nilVal();
}
Value engineDrawSpriteEx(const std::vector<Value>& args) {
    if (args.size() >= 8 && g_Renderer) {
        int id = (int)args[0].numberVal;
        float x = args[1].numberVal;
        float y = args[2].numberVal;
        float rot = args[3].numberVal;
        float sx = args[4].numberVal;
        float sy = args[5].numberVal;
        bool fx = args[6].isBool() ? args[6].boolVal : (args[6].numberVal != 0);
        bool fy = args[7].isBool() ? args[7].boolVal : (args[7].numberVal != 0);
        float ox = -1.0f;
        float oy = -1.0f;
        float alpha = 1.0f;
        if (args.size() >= 9) {
            ox = args[8].numberVal;
            oy = (args.size() >= 10) ? args[9].numberVal : ox;
        }
        if (args.size() >= 11) {
            alpha = args[10].numberVal;
        }
        applySpriteOverrides(id, x, y, rot, sx, sy, alpha);
        g_Renderer->drawSpriteEx(id, x, y, rot, sx, sy, fx, fy, ox, oy, alpha);
    }
    return Value::nilVal();
}
Value engineDrawSpriteFrame(const std::vector<Value>& args) {
    if (args.size() < 9 || !g_Renderer) return Value::nilVal();
    int sheetId = (int)args[0].numberVal;
    int frame = (int)args[1].numberVal;
    float x = args[2].numberVal;
    float y = args[3].numberVal;
    float rot = args[4].numberVal;
    float sx = args[5].numberVal;
    float sy = args[6].numberVal;
    bool fx = args[7].isBool() ? args[7].boolVal : (args[7].numberVal != 0);
    bool fy = args[8].isBool() ? args[8].boolVal : (args[8].numberVal != 0);
    float ox = -1.0f;
    float oy = -1.0f;
    float alpha = 1.0f;
    if (args.size() >= 11) {
        ox = args[9].numberVal;
        oy = args[10].numberVal;
    } else if (args.size() == 10) {
        ox = args[9].numberVal;
    }
    if (args.size() >= 12) {
        alpha = args[11].numberVal;
    }
    g_Renderer->drawSpriteFrame(sheetId, frame, x, y, rot, sx, sy, fx, fy, ox, oy, alpha);
    return Value::nilVal();
}
Value engineAnimCreate(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::number(-1);
    int sheetId = (int)args[0].numberVal;
    std::vector<int> frames = toFrameList(args[1]);
    if (frames.empty()) return Value::number(-1);
    double fps = args[2].numberVal;
    bool loop = true;
    if (args.size() >= 4) {
        loop = args[3].isBool() ? args[3].boolVal : (args[3].numberVal != 0);
    }
    Animation anim;
    anim.id = g_AnimationCounter++;
    anim.sheetId = sheetId;
    anim.frames = frames;
    anim.fps = fps;
    anim.loop = loop;
    anim.playing = true;
    g_Animations[anim.id] = anim;
    return Value::number(anim.id);
}
Value engineAnimPlay(const std::vector<Value>& args) {
    if (args.empty()) return Value::nilVal();
    int id = (int)args[0].numberVal;
    auto* anim = getAnimation(id);
    if (!anim) return Value::nilVal();
    bool restart = args.size() >= 2 ? (args[1].isBool() ? args[1].boolVal : (args[1].numberVal != 0)) : false;
    if (restart) {
        anim->currentIndex = 0;
        anim->accumulator = 0.0;
    }
    anim->playing = true;
    return Value::nilVal();
}
Value engineAnimStop(const std::vector<Value>& args) {
    if (args.empty()) return Value::nilVal();
    auto* anim = getAnimation((int)args[0].numberVal);
    if (anim) anim->playing = false;
    return Value::nilVal();
}
Value engineAnimReset(const std::vector<Value>& args) {
    if (args.empty()) return Value::nilVal();
    auto* anim = getAnimation((int)args[0].numberVal);
    if (!anim) return Value::nilVal();
    anim->currentIndex = 0;
    anim->accumulator = 0.0;
    return Value::nilVal();
}
Value engineAnimSetPosition(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::nilVal();
    auto* anim = getAnimation((int)args[0].numberVal);
    if (!anim) return Value::nilVal();
    anim->transform.x = args[1].numberVal;
    anim->transform.y = args[2].numberVal;
    return Value::nilVal();
}
Value engineAnimSetOrigin(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::nilVal();
    auto* anim = getAnimation((int)args[0].numberVal);
    if (!anim) return Value::nilVal();
    anim->transform.originX = args[1].numberVal;
    anim->transform.originY = args[2].numberVal;
    return Value::nilVal();
}
Value engineAnimSetScale(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::nilVal();
    auto* anim = getAnimation((int)args[0].numberVal);
    if (!anim) return Value::nilVal();
    anim->transform.scaleX = args[1].numberVal;
    anim->transform.scaleY = args[2].numberVal;
    return Value::nilVal();
}
Value engineAnimSetRotation(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::nilVal();
    auto* anim = getAnimation((int)args[0].numberVal);
    if (!anim) return Value::nilVal();
    anim->transform.rotationDeg = args[1].numberVal;
    return Value::nilVal();
}
Value engineAnimSetFlip(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::nilVal();
    auto* anim = getAnimation((int)args[0].numberVal);
    if (!anim) return Value::nilVal();
    anim->transform.flipX = args[1].isBool() ? args[1].boolVal : (args[1].numberVal != 0);
    anim->transform.flipY = args[2].isBool() ? args[2].boolVal : (args[2].numberVal != 0);
    return Value::nilVal();
}
Value engineAnimSetAlpha(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::nilVal();
    auto* anim = getAnimation((int)args[0].numberVal);
    if (!anim) return Value::nilVal();
    anim->alpha = args[1].numberVal;
    return Value::nilVal();
}
Value engineAnimDraw(const std::vector<Value>& args) {
    if (args.empty() || !g_Renderer) return Value::nilVal();
    auto* anim = getAnimation((int)args[0].numberVal);
    if (!anim || anim->frames.empty()) return Value::nilVal();
    int frame = anim->frames[anim->currentIndex % (int)anim->frames.size()];
    g_Renderer->drawSpriteFrame(anim->sheetId, frame, anim->transform.x, anim->transform.y, anim->transform.rotationDeg, anim->transform.scaleX, anim->transform.scaleY, anim->transform.flipX, anim->transform.flipY, anim->transform.originX, anim->transform.originY, anim->alpha);
    return Value::nilVal();
}
Value engineColliderCreate(const std::vector<Value>& args) {
    if (args.size() < 4) return Value::number(-1);
    Collider c;
    c.x = (float)args[0].numberVal;
    c.y = (float)args[1].numberVal;
    c.w = (float)args[2].numberVal;
    c.h = (float)args[3].numberVal;
    if (args.size() >= 5 && args[4].isString()) c.tag = args[4].stringVal;
    if (args.size() >= 6) c.solid = args[5].isBool() ? args[5].boolVal : (args[5].numberVal != 0);
    g_Colliders.push_back(c);
    return Value::number((int)g_Colliders.size() - 1);
}
Value engineColliderSetPos(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::nilVal();
    int id = (int)args[0].numberVal;
    if (id < 0 || id >= (int)g_Colliders.size()) return Value::nilVal();
    g_Colliders[id].x = (float)args[1].numberVal;
    g_Colliders[id].y = (float)args[2].numberVal;
    return Value::nilVal();
}
Value engineColliderSetSize(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::nilVal();
    int id = (int)args[0].numberVal;
    if (id < 0 || id >= (int)g_Colliders.size()) return Value::nilVal();
    g_Colliders[id].w = (float)args[1].numberVal;
    g_Colliders[id].h = (float)args[2].numberVal;
    return Value::nilVal();
}
Value engineColliderGetPos(const std::vector<Value>& args) {
    if (args.empty()) return Value::map({});
    int id = (int)args[0].numberVal;
    if (id < 0 || id >= (int)g_Colliders.size()) return Value::map({});
    std::unordered_map<std::string, Value> m;
    m["x"] = Value::number(g_Colliders[id].x);
    m["y"] = Value::number(g_Colliders[id].y);
    return Value::map(m);
}
Value engineColliderGetSize(const std::vector<Value>& args) {
    if (args.empty()) return Value::map({});
    int id = (int)args[0].numberVal;
    if (id < 0 || id >= (int)g_Colliders.size()) return Value::map({});
    std::unordered_map<std::string, Value> m;
    m["w"] = Value::number(g_Colliders[id].w);
    m["h"] = Value::number(g_Colliders[id].h);
    return Value::map(m);
}
Value engineColliderMove(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::array({});
    int id = (int)args[0].numberVal;
    if (id < 0 || id >= (int)g_Colliders.size()) return Value::array({});
    float dx = (float)args[1].numberVal;
    float dy = (float)args[2].numberVal;
    Collider& c = g_Colliders[id];
    float nx = c.x + dx;
    float ny = c.y + dy;
    std::set<int> hits;
    for (size_t i = 0; i < g_Colliders.size(); ++i) {
        if ((int)i == id) continue;
        const Collider& o = g_Colliders[i];
        if (!rectsOverlap(nx, c.y, c.w, c.h, o.x, o.y, o.w, o.h)) continue;
        if (c.solid && o.solid) {
            if (dx > 0.0f) nx = std::min(nx, o.x - c.w);
            else if (dx < 0.0f) nx = std::max(nx, o.x + o.w);
        }
        hits.insert((int)i);
    }
    c.x = nx;
    for (size_t i = 0; i < g_Colliders.size(); ++i) {
        if ((int)i == id) continue;
        const Collider& o = g_Colliders[i];
        if (!rectsOverlap(c.x, ny, c.w, c.h, o.x, o.y, o.w, o.h)) continue;
        if (c.solid && o.solid) {
            if (dy > 0.0f) ny = std::min(ny, o.y - c.h);
            else if (dy < 0.0f) ny = std::max(ny, o.y + o.h);
        }
        hits.insert((int)i);
    }
    c.y = ny;

    std::vector<Value> arr;
    arr.reserve(hits.size());
    for (int hid : hits) {
        if (hid < 0 || hid >= (int)g_Colliders.size()) continue;
        std::unordered_map<std::string, Value> m;
        m["id"] = Value::number(hid);
        m["tag"] = Value::string(g_Colliders[hid].tag);
        arr.push_back(Value::map(m));
    }
    return Value::array(arr);
}
Value engineDrawText(const std::vector<Value>& args) {
    if (args.size() < 4 || !g_Renderer) return Value::nilVal();
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
            break;
        }
    }
    if (!usedKv) {
        if (args.size() >= 5 && args[4].isNumber()) scale = args[4].numberVal;
        if (args.size() >= 6 && args[5].isNumber()) r = args[5].numberVal;
        if (args.size() >= 7 && args[6].isNumber()) g = args[6].numberVal;
        if (args.size() >= 8 && args[7].isNumber()) b = args[7].numberVal;
        if (args.size() >= 9 && args[8].isNumber()) a = args[8].numberVal;
        if (args.size() >= 10 && args[9].isString()) align = args[9].toString();
        if (args.size() >= 11 && args[10].isNumber()) maxWidth = args[10].numberVal;
        if (args.size() >= 12 && args[11].isNumber()) lineHeight = args[11].numberVal;
    }
    g_Renderer->drawTextEx(fontId, text, x, y, scale, r, g, b, a, align, maxWidth, lineHeight);
    return Value::nilVal();
}
Value engineMeasureTextWidth(const std::vector<Value>& args) {
    if (args.size() < 2 || !g_Renderer) return Value::number(0);
    int fontId = (int)args[0].numberVal;
    std::string text = args[1].toString();
    float scale = 1.0f;
    float maxWidth = 0.0f;
    float lineHeight = 0.0f;
    size_t i = 2;
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
            break;
        }
    }
    if (!usedKv) {
        if (args.size() > 2 && args[2].isNumber()) scale = args[2].numberVal;
        if (args.size() > 3 && args[3].isNumber()) maxWidth = args[3].numberVal;
        if (args.size() > 4 && args[4].isNumber()) lineHeight = args[4].numberVal;
    }
    return Value::number(g_Renderer->measureTextWidth(fontId, text, scale, maxWidth, lineHeight));
}
Value engineMeasureTextHeight(const std::vector<Value>& args) {
    if (args.size() < 2 || !g_Renderer) return Value::number(0);
    int fontId = (int)args[0].numberVal;
    std::string text = args[1].toString();
    float scale = 1.0f;
    float maxWidth = 0.0f;
    float lineHeight = 0.0f;
    size_t i = 2;
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
            break;
        }
    }
    if (!usedKv) {
        if (args.size() > 2 && args[2].isNumber()) scale = args[2].numberVal;
        if (args.size() > 3 && args[3].isNumber()) maxWidth = args[3].numberVal;
        if (args.size() > 4 && args[4].isNumber()) lineHeight = args[4].numberVal;
    }
    return Value::number(g_Renderer->measureTextHeight(fontId, text, scale, maxWidth, lineHeight));
}
Value engineSetDebugDrawEnabled(const std::vector<Value>& args) {
    if (args.size() >= 1 && g_Renderer) {
        bool on = args[0].isBool() ? args[0].boolVal : (args[0].numberVal != 0);
        g_Renderer->setDebugEnabled(on);
    }
    return Value::nilVal();
}
Value engineDebugDrawRect(const std::vector<Value>& args) {
    if (args.size() >= 7 && g_Renderer) {
        g_Renderer->debugDrawRect(args[0].numberVal, args[1].numberVal, args[2].numberVal, args[3].numberVal, args[4].numberVal, args[5].numberVal, args[6].numberVal);
    }
    return Value::nilVal();
}
Value engineDebugDrawLine(const std::vector<Value>& args) {
    if (args.size() >= 7 && g_Renderer) {
        g_Renderer->debugDrawLine(args[0].numberVal, args[1].numberVal, args[2].numberVal, args[3].numberVal, args[4].numberVal, args[5].numberVal, args[6].numberVal);
    }
    return Value::nilVal();
}
Value engineSin(const std::vector<Value>& args) {
    if (args.size() >= 1 && args[0].isNumber()) {
        return Value::number(std::sin(args[0].numberVal));
    }
    return Value::number(0);
}
Value engineCos(const std::vector<Value>& args) {
    if (args.size() >= 1 && args[0].isNumber()) {
        return Value::number(std::cos(args[0].numberVal));
    }
    return Value::number(0);
}
Value engineIsKeyDown(const std::vector<Value>& args) {
    if (args.empty()) return Value::boolean(false);
    int key = resolveKey(args[0]);
    if (key >= 0) {
        return Value::boolean(isKeyDown(key));
    }
    return Value::boolean(false);
}
Value engineIsKeyPressed(const std::vector<Value>& args) {
    if (args.empty()) return Value::boolean(false);
    int key = resolveKey(args[0]);
    if (key >= 0) {
        return Value::boolean(isKeyPressed(key));
    }
    return Value::boolean(false);
}
Value engineIsKeyReleased(const std::vector<Value>& args) {
    if (args.empty()) return Value::boolean(false);
    int key = resolveKey(args[0]);
    if (key >= 0) {
        return Value::boolean(isKeyReleased(key));
    }
    return Value::boolean(false);
}
Value engineIsMouseDown(const std::vector<Value>& args) {
    if (args.empty()) return Value::boolean(false);
    int btn = resolveMouseButton(args[0]);
    if (btn >= 0) {
        return Value::boolean(isMouseDown(btn));
    }
    return Value::boolean(false);
}
Value engineIsMousePressed(const std::vector<Value>& args) {
    if (args.empty()) return Value::boolean(false);
    int btn = resolveMouseButton(args[0]);
    if (btn >= 0) {
        return Value::boolean(isMousePressed(btn));
    }
    return Value::boolean(false);
}
Value engineIsMouseReleased(const std::vector<Value>& args) {
    if (args.empty()) return Value::boolean(false);
    int btn = resolveMouseButton(args[0]);
    if (btn >= 0) {
        return Value::boolean(isMouseReleased(btn));
    }
    return Value::boolean(false);
}
Value engineGetMouseX(const std::vector<Value>&) {
    return Value::number(getMouseX());
}
Value engineGetMouseY(const std::vector<Value>&) {
    return Value::number(getMouseY());
}
Value engineBindAction(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::nilVal();
    std::string name = args[0].toString();
    auto binding = resolveActionBinding(args[1]);
    if (binding.second < 0) return Value::nilVal();
    bindAction(name, binding.first, binding.second);
    return Value::nilVal();
}
Value engineUnbindAction(const std::vector<Value>& args) {
    if (args.empty()) return Value::nilVal();
    std::string name = args[0].toString();
    unbindAction(name);
    return Value::nilVal();
}
Value engineActionDown(const std::vector<Value>& args) {
    if (args.empty()) return Value::boolean(false);
    return Value::boolean(isActionDown(args[0].toString()));
}
Value engineActionPressed(const std::vector<Value>& args) {
    if (args.empty()) return Value::boolean(false);
    return Value::boolean(isActionPressed(args[0].toString()));
}
Value engineActionReleased(const std::vector<Value>& args) {
    if (args.empty()) return Value::boolean(false);
    return Value::boolean(isActionReleased(args[0].toString()));
}
Value engineTweenValue(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::number(-1);
    double from = args[0].numberVal;
    double to = args[1].numberVal;
    double duration = args[2].numberVal;
    std::string easing = args.size() > 3 ? args[3].toString() : "linear";
    return Value::number(createValueTween(from, to, duration, easing));
}
Value engineTweenValueGet(const std::vector<Value>& args) {
    if (args.empty()) return Value::nilVal();
    int id = (int)args[0].numberVal;
    auto it = g_Tweens.find(id);
    if (it == g_Tweens.end()) return Value::nilVal();
    return Value::number(it->second.current);
}
Value engineTweenProperty(const std::vector<Value>& args) {
    if (args.size() < 4) return Value::number(-1);
    TweenTarget tgt = parseTarget(args[0]);
    std::string prop = args[1].toString();
    double to = args[2].numberVal;
    double duration = args[3].numberVal;
    std::string easing = args.size() > 4 ? args[4].toString() : "linear";
    int id = createPropertyTween(tgt, prop, to, duration, easing);
    return Value::number(id);
}
Value engineTweenSequenceStart(const std::vector<Value>&) {
    int id = nextSequenceId();
    Sequence seq;
    seq.id = id;
    g_Sequences[id] = seq;
    return Value::number(id);
}
Value engineTweenSequenceAdd(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::nilVal();
    int seqId = (int)args[0].numberVal;
    int tweenId = (int)args[1].numberVal;
    auto it = g_Sequences.find(seqId);
    auto tw = g_Tweens.find(tweenId);
    if (it == g_Sequences.end() || tw == g_Tweens.end()) return Value::nilVal();
    it->second.tweens.push_back(tweenId);
    resetTween(tw->second, true, true);
    g_TweenSequenceOwner[tweenId] = seqId;
    return Value::nilVal();
}
Value engineTweenSequencePlay(const std::vector<Value>& args) {
    if (args.empty()) return Value::nilVal();
    int seqId = (int)args[0].numberVal;
    auto it = g_Sequences.find(seqId);
    if (it == g_Sequences.end()) return Value::nilVal();
    it->second.playing = true;
    it->second.finished = false;
    it->second.currentIndex = 0;
    for (int tid : it->second.tweens) {
        auto tw = g_Tweens.find(tid);
        if (tw != g_Tweens.end()) resetTween(tw->second, false, true);
    }
    return Value::nilVal();
}
Value engineTweenParallelStart(const std::vector<Value>&) {
    int id = nextParallelId();
    ParallelGroup grp;
    grp.id = id;
    g_Parallels[id] = grp;
    return Value::number(id);
}
Value engineTweenParallelAdd(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::nilVal();
    int grpId = (int)args[0].numberVal;
    int tweenId = (int)args[1].numberVal;
    auto it = g_Parallels.find(grpId);
    auto tw = g_Tweens.find(tweenId);
    if (it == g_Parallels.end() || tw == g_Tweens.end()) return Value::nilVal();
    it->second.tweens.push_back(tweenId);
    resetTween(tw->second, true, true);
    g_TweenParallelOwner[tweenId] = grpId;
    return Value::nilVal();
}
Value engineTweenParallelPlay(const std::vector<Value>& args) {
    if (args.empty()) return Value::nilVal();
    int grpId = (int)args[0].numberVal;
    auto it = g_Parallels.find(grpId);
    if (it == g_Parallels.end()) return Value::nilVal();
    it->second.playing = true;
    it->second.finished = false;
    for (int tid : it->second.tweens) {
        auto tw = g_Tweens.find(tid);
        if (tw != g_Tweens.end()) resetTween(tw->second, false, true);
    }
    return Value::nilVal();
}
Value engineTweenPause(const std::vector<Value>& args) {
    if (args.empty()) return Value::nilVal();
    int id = (int)args[0].numberVal;
    auto it = g_Tweens.find(id);
    if (it != g_Tweens.end()) it->second.paused = true;
    return Value::nilVal();
}
Value engineTweenResume(const std::vector<Value>& args) {
    if (args.empty()) return Value::nilVal();
    int id = (int)args[0].numberVal;
    auto it = g_Tweens.find(id);
    if (it != g_Tweens.end()) it->second.paused = false;
    return Value::nilVal();
}
Value engineTweenCancel(const std::vector<Value>& args) {
    if (args.empty()) return Value::nilVal();
    int id = (int)args[0].numberVal;
    auto it = g_Tweens.find(id);
    if (it != g_Tweens.end()) {
        it->second.canceled = true;
        finishTween(it->second);
    }
    return Value::nilVal();
}
Value engineTweenOnComplete(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::nilVal();
    int id = (int)args[0].numberVal;
    auto it = g_Tweens.find(id);
    if (it == g_Tweens.end()) return Value::nilVal();
    it->second.onComplete = args[1];
    return Value::nilVal();
}
Value engineShake(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::number(-1);
    TweenTarget tgt = parseTarget(args[0]);
    float baseX = 0.0f;
    float baseY = 0.0f;
    getPropertyValue(tgt, "x", baseX);
    getPropertyValue(tgt, "y", baseY);
    double magnitude = args[1].numberVal;
    double duration = args[2].numberVal;
    int steps = 6;
    int seqId = nextSequenceId();
    Sequence s;
    s.id = seqId;
    double stepDur = duration / steps;
    for (int i = 0; i < steps; ++i) {
        double ox = ((double(rand()) / RAND_MAX) * 2.0 - 1.0) * magnitude;
        double oy = ((double(rand()) / RAND_MAX) * 2.0 - 1.0) * magnitude;
        int tx = createPropertyTween(tgt, "x", baseX + ox, stepDur, "linear");
        int ty = createPropertyTween(tgt, "y", baseY + oy, stepDur, "linear");
        s.tweens.push_back(tx);
        s.tweens.push_back(ty);
        g_TweenSequenceOwner[tx] = seqId;
        g_TweenSequenceOwner[ty] = seqId;
    }
    int txFinal = createPropertyTween(tgt, "x", baseX, stepDur, "linear");
    int tyFinal = createPropertyTween(tgt, "y", baseY, stepDur, "linear");
    s.tweens.push_back(txFinal);
    s.tweens.push_back(tyFinal);
    g_TweenSequenceOwner[txFinal] = seqId;
    g_TweenSequenceOwner[tyFinal] = seqId;
    s.playing = true;
    g_Sequences[seqId] = s;
    return Value::number(seqId);
}
Value engineSquash(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::number(-1);
    TweenTarget tgt = parseTarget(args[0]);
    double amount = args[1].numberVal;
    double duration = args[2].numberVal;
    float baseX = 1.0f, baseY = 1.0f;
    getPropertyValue(tgt, "scale_x", baseX);
    getPropertyValue(tgt, "scale_y", baseY);
    int seqId = nextSequenceId();
    Sequence s;
    s.id = seqId;
    int downX = createPropertyTween(tgt, "scale_x", baseX + amount, duration * 0.5, "ease_out");
    int downY = createPropertyTween(tgt, "scale_y", baseY - amount, duration * 0.5, "ease_out");
    int upX = createPropertyTween(tgt, "scale_x", baseX, duration * 0.5, "ease_in");
    int upY = createPropertyTween(tgt, "scale_y", baseY, duration * 0.5, "ease_in");
    s.tweens.push_back(downX);
    s.tweens.push_back(downY);
    s.tweens.push_back(upX);
    s.tweens.push_back(upY);
    g_TweenSequenceOwner[downX] = seqId;
    g_TweenSequenceOwner[downY] = seqId;
    g_TweenSequenceOwner[upX] = seqId;
    g_TweenSequenceOwner[upY] = seqId;
    s.playing = true;
    g_Sequences[seqId] = s;
    return Value::number(seqId);
}
Value engineBounce(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::number(-1);
    TweenTarget tgt = parseTarget(args[0]);
    double height = args[1].numberVal;
    double duration = args[2].numberVal;
    float baseY = 0.0f;
    getPropertyValue(tgt, "y", baseY);
    int seqId = nextSequenceId();
    Sequence s;
    s.id = seqId;
    int up = createPropertyTween(tgt, "y", baseY - height, duration * 0.5, "ease_out");
    int down = createPropertyTween(tgt, "y", baseY, duration * 0.5, "ease_in");
    s.tweens.push_back(up);
    s.tweens.push_back(down);
    g_TweenSequenceOwner[up] = seqId;
    g_TweenSequenceOwner[down] = seqId;
    s.playing = true;
    g_Sequences[seqId] = s;
    return Value::number(seqId);
}
Value engineFlash(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::number(-1);
    TweenTarget tgt = parseTarget(args[0]);
    int times = (int)args[1].numberVal;
    double duration = args[2].numberVal;
    float baseA = 1.0f;
    getPropertyValue(tgt, "alpha", baseA);
    int seqId = nextSequenceId();
    Sequence s;
    s.id = seqId;
    double step = duration / (times * 2);
    for (int i = 0; i < times; ++i) {
        int off = createPropertyTween(tgt, "alpha", 0.0, step, "linear");
        int on = createPropertyTween(tgt, "alpha", baseA, step, "linear");
        s.tweens.push_back(off);
        s.tweens.push_back(on);
        g_TweenSequenceOwner[off] = seqId;
        g_TweenSequenceOwner[on] = seqId;
    }
    s.playing = true;
    g_Sequences[seqId] = s;
    return Value::number(seqId);
}
Value engineRectOverlaps(const std::vector<Value>& args) {
    if (args.size() < 8) return Value::boolean(false);
    float x1 = args[0].numberVal;
    float y1 = args[1].numberVal;
    float w1 = args[2].numberVal;
    float h1 = args[3].numberVal;
    float x2 = args[4].numberVal;
    float y2 = args[5].numberVal;
    float w2 = args[6].numberVal;
    float h2 = args[7].numberVal;
    bool overlap = x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2;
    return Value::boolean(overlap);
}
Value enginePointInRect(const std::vector<Value>& args) {
    if (args.size() < 6) return Value::boolean(false);
    float px = args[0].numberVal;
    float py = args[1].numberVal;
    float rx = args[2].numberVal;
    float ry = args[3].numberVal;
    float rw = args[4].numberVal;
    float rh = args[5].numberVal;
    bool inside = px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
    return Value::boolean(inside);
}
Value engineCreateAreaRect(const std::vector<Value>& args) {
    if (args.size() < 5) return Value::number(-1);
    Area a;
    a.x = args[0].numberVal;
    a.y = args[1].numberVal;
    a.w = args[2].numberVal;
    a.h = args[3].numberVal;
    a.tag = args[4].toString();
    g_Areas.push_back(a);
    return Value::number((int)g_Areas.size() - 1);
}
Value engineSetAreaRect(const std::vector<Value>& args) {
    if (args.size() < 5) return Value::nilVal();
    int id = (int)args[0].numberVal;
    if (id < 0 || id >= (int)g_Areas.size()) return Value::nilVal();
    g_Areas[id].x = args[1].numberVal;
    g_Areas[id].y = args[2].numberVal;
    g_Areas[id].w = args[3].numberVal;
    g_Areas[id].h = args[4].numberVal;
    return Value::nilVal();
}
Value engineAreaOverlaps(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::boolean(false);
    int a = (int)args[0].numberVal;
    int b = (int)args[1].numberVal;
    if (a < 0 || b < 0 || a >= (int)g_Areas.size() || b >= (int)g_Areas.size()) return Value::boolean(false);
    const auto& A = g_Areas[a];
    const auto& B = g_Areas[b];
    bool overlap = A.x < B.x + B.w && A.x + A.w > B.x && A.y < B.y + B.h && A.y + A.h > B.y;
    return Value::boolean(overlap);
}
Value engineDebugArea(const std::vector<Value>& args) {
    if (args.size() < 2 || !g_Renderer) return Value::nilVal();
    int id = (int)args[0].numberVal;
    float r = args[1].numberVal;
    float g = args.size() > 2 ? args[2].numberVal : 1.0f;
    float b = args.size() > 3 ? args[3].numberVal : 0.0f;
    if (id >= 0 && id < (int)g_Areas.size()) {
        const auto& A = g_Areas[id];
        g_Renderer->debugDrawRect(A.x, A.y, A.w, A.h, r, g, b);
    }
    return Value::nilVal();
}
static std::string makeAreaKey(int id, const std::string& tag) {
    return std::to_string(id) + "|" + tag;
}
static bool areaOverlapsTagInternal(int id, const std::string& tag) {
    if (id < 0 || id >= (int)g_Areas.size()) return false;
    const auto& A = g_Areas[id];
    for (size_t i = 0; i < g_Areas.size(); ++i) {
        if ((int)i == id) continue;
        if (g_Areas[i].tag != tag) continue;
        const auto& B = g_Areas[i];
        bool overlap = A.x < B.x + B.w && A.x + A.w > B.x && A.y < B.y + B.h && A.y + A.h > B.y;
        if (overlap) return true;
    }
    return false;
}
Value engineAreaOverlapsTag(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::boolean(false);
    int id = (int)args[0].numberVal;
    std::string tag = args[1].toString();
    return Value::boolean(areaOverlapsTagInternal(id, tag));
}
Value engineAreaEnteredTag(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::boolean(false);
    int id = (int)args[0].numberVal;
    std::string tag = args[1].toString();
    std::string key = makeAreaKey(id, tag);
    bool now = areaOverlapsTagInternal(id, tag);
    bool before = g_AreaState[key];
    g_AreaState[key] = now;
    return Value::boolean(now && !before);
}
Value engineAreaExitedTag(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::boolean(false);
    int id = (int)args[0].numberVal;
    std::string tag = args[1].toString();
    std::string key = makeAreaKey(id, tag);
    bool now = areaOverlapsTagInternal(id, tag);
    bool before = g_AreaState[key];
    g_AreaState[key] = now;
    return Value::boolean(!now && before);
}
void EngineBindings::registerBuiltins(std::unordered_map<std::string, NativeFn>& builtins) {
    builtins["engine_log"] = engineLog;
    builtins["import"] = engineImport;
    builtins["time"] = engineTime;
    builtins["random"] = engineRandom;
    builtins["set_clear_color"] = engineSetClearColor;
    builtins["draw_rect"] = engineDrawRect;
    builtins["load_sprite"] = engineLoadSprite;
    builtins["load_sprite_sheet"] = engineLoadSpriteSheet;
    builtins["draw_sprite"] = engineDrawSprite;
    builtins["draw_sprite_ex"] = engineDrawSpriteEx;
    builtins["draw_sprite_frame"] = engineDrawSpriteFrame;
    builtins["load_font"] = engineLoadFont;
    builtins["draw_text"] = engineDrawText;
    builtins["draw_text_ex"] = engineDrawText;
    builtins["measure_text_width"] = engineMeasureTextWidth;
    builtins["measure_text_height"] = engineMeasureTextHeight;
    builtins["anim_create"] = engineAnimCreate;
    builtins["anim_play"] = engineAnimPlay;
    builtins["anim_stop"] = engineAnimStop;
    builtins["anim_reset"] = engineAnimReset;
    builtins["anim_set_position"] = engineAnimSetPosition;
    builtins["anim_set_origin"] = engineAnimSetOrigin;
    builtins["anim_set_scale"] = engineAnimSetScale;
    builtins["anim_set_rotation"] = engineAnimSetRotation;
    builtins["anim_set_flip"] = engineAnimSetFlip;
    builtins["anim_set_alpha"] = engineAnimSetAlpha;
    builtins["anim_draw"] = engineAnimDraw;
    builtins["collider_create"] = engineColliderCreate;
    builtins["collider_set_position"] = engineColliderSetPos;
    builtins["collider_set_size"] = engineColliderSetSize;
    builtins["collider_get_position"] = engineColliderGetPos;
    builtins["collider_get_size"] = engineColliderGetSize;
    builtins["collider_move"] = engineColliderMove;
    builtins["set_debug_draw_enabled"] = engineSetDebugDrawEnabled;
    builtins["debug_draw_rect"] = engineDebugDrawRect;
    builtins["debug_draw_line"] = engineDebugDrawLine;
    builtins["sin"] = engineSin;
    builtins["cos"] = engineCos;
    builtins["is_key_down"] = engineIsKeyDown;
    builtins["is_key_pressed"] = engineIsKeyPressed;
    builtins["is_key_released"] = engineIsKeyReleased;
    builtins["is_mouse_down"] = engineIsMouseDown;
    builtins["is_mouse_pressed"] = engineIsMousePressed;
    builtins["is_mouse_released"] = engineIsMouseReleased;
    builtins["get_mouse_x"] = engineGetMouseX;
    builtins["get_mouse_y"] = engineGetMouseY;
    builtins["bind_action"] = engineBindAction;
    builtins["unbind_action"] = engineUnbindAction;
    builtins["action_down"] = engineActionDown;
    builtins["action_pressed"] = engineActionPressed;
    builtins["action_released"] = engineActionReleased;
    builtins["tween_value"] = engineTweenValue;
    builtins["tween_value_get"] = engineTweenValueGet;
    builtins["tween_property"] = engineTweenProperty;
    builtins["tween_sequence_start"] = engineTweenSequenceStart;
    builtins["tween_sequence_add"] = engineTweenSequenceAdd;
    builtins["tween_sequence_play"] = engineTweenSequencePlay;
    builtins["tween_parallel_start"] = engineTweenParallelStart;
    builtins["tween_parallel_add"] = engineTweenParallelAdd;
    builtins["tween_parallel_play"] = engineTweenParallelPlay;
    builtins["tween_pause"] = engineTweenPause;
    builtins["tween_resume"] = engineTweenResume;
    builtins["tween_cancel"] = engineTweenCancel;
    builtins["tween_on_complete"] = engineTweenOnComplete;
    builtins["shake"] = engineShake;
    builtins["squash"] = engineSquash;
    builtins["bounce"] = engineBounce;
    builtins["flash"] = engineFlash;
    builtins["rect_overlaps"] = engineRectOverlaps;
    builtins["point_in_rect"] = enginePointInRect;
    builtins["get_screen_size"] = [](const std::vector<Value>&)->Value {
        if (!g_Window) return Value::map({});
        int w = 0, h = 0;
        g_Window->getFramebufferSize(w, h);
        std::unordered_map<std::string, Value> m;
        m["w"] = Value::number((double)w);
        m["h"] = Value::number((double)h);
        return Value::map(m);
    };
    builtins["create_area_rect"] = engineCreateAreaRect;
    builtins["set_area_rect"] = engineSetAreaRect;
    builtins["area_overlaps"] = engineAreaOverlaps;
    builtins["area_overlaps_tag"] = engineAreaOverlapsTag;
    builtins["area_entered_tag"] = engineAreaEnteredTag;
    builtins["area_exited_tag"] = engineAreaExitedTag;
    builtins["debug_area"] = engineDebugArea;
}
} // namespace yuki
