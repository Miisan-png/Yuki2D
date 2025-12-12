#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <limits>
#include "../renderer2d.hpp"
#include "../window.hpp"
#include "../../script/value.hpp"
#include "../../script/interpreter.hpp"

namespace yuki {
struct Area {
    float x, y, w, h;
    std::string tag;
};

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

struct Collider {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
    std::string tag;
    bool solid = true;
};

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

struct BindingsState {
    Window* window = nullptr;
    Renderer2D* renderer = nullptr;
    Interpreter* interpreter = nullptr;
    std::string assetBase;
    std::vector<std::filesystem::path> importPaths;
    std::unordered_set<std::string> loadedModules;

    std::vector<Area> areas;
    std::unordered_map<std::string, bool> areaState;

    std::unordered_map<int, SpriteState> spriteStates;
    std::unordered_map<int, Animation> animations;
    int animationCounter = 1;

    std::vector<Collider> colliders;

    std::unordered_map<int, Tween> tweens;
    std::unordered_map<int, Sequence> sequences;
    std::unordered_map<int, ParallelGroup> parallels;
    std::unordered_map<int, int> tweenSequenceOwner;
    std::unordered_map<int, int> tweenParallelOwner;
    int tweenCounter = 1;
    int sequenceCounter = 1;
    int parallelCounter = 1;

    struct AseAsset {
        int id = -1;
        int sheetId = -1;
        int frameW = 0;
        int frameH = 0;
        std::string path;
        std::filesystem::file_time_type lastWriteTime;
        std::unordered_map<std::string, std::vector<int>> tagFrames;
        std::unordered_map<std::string, double> tagFps;
    };
    std::unordered_map<int, AseAsset> aseAssets;
    int aseCounter = 1;
};

BindingsState& bindingsState();
void resetBindingsState();
} // namespace yuki
