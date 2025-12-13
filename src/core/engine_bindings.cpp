#include "engine_bindings.hpp"
#include "bindings/state.hpp"
#include "bindings/register_bindings.hpp"
#include "bindings/anim_api.hpp"
#include "bindings/tween_api.hpp"
#include "aseprite_loader.hpp"
#include "log.hpp"
#include <filesystem>
#include <functional>
#include <algorithm>
#include <cmath>

namespace yuki {
namespace {
BindingsState& st = bindingsState();

void hotReloadAse(double dt) {
    static double accum = 0.0;
    accum += dt;
    if (accum < 0.25) return;
    accum = 0.0;
    for (auto& kv : st.aseAssets) {
        auto& asset = kv.second;
        if (asset.path.empty()) continue;
        std::error_code ec;
        auto cur = std::filesystem::last_write_time(asset.path, ec);
        if (ec || cur <= asset.lastWriteTime) continue;
        AseData data;
        if (!loadAsepriteFile(asset.path, data)) continue;
        if (data.width <= 0 || data.height <= 0 || data.frames.empty()) continue;
        if (!st.renderer->updateSpriteSheetFromFrames(asset.sheetId, data.width, data.height, data.frames)) continue;
        asset.frameW = data.width;
        asset.frameH = data.height;
        asset.lastWriteTime = cur;
        asset.tagFrames.clear();
        asset.tagFps.clear();
        for (const auto& tag : data.tags) {
            std::vector<int> frames;
            for (int fi = tag.from; fi <= tag.to && fi < (int)data.frames.size(); ++fi) {
                frames.push_back(fi);
            }
            if (tag.direction == 1) std::reverse(frames.begin(), frames.end());
            else if (tag.direction == 2) {
                if (!frames.empty()) {
                    for (int i = (int)frames.size() - 2; i >= 1; --i) frames.push_back(frames[i]);
                }
            }
            if (frames.empty()) continue;
            double avgMs = 0.0;
            int count = 0;
            for (int f : frames) {
                if (f >= 0 && f < (int)data.durationsMs.size() && data.durationsMs[f] > 0) {
                    avgMs += data.durationsMs[f];
                    count++;
                }
            }
            double fps = 10.0;
            if (count > 0) {
                avgMs /= (double)count;
                fps = avgMs > 0.0 ? 1000.0 / avgMs : 10.0;
            }
            asset.tagFrames[tag.name] = frames;
            asset.tagFps[tag.name] = fps;
        }
        logInfo("ase_reload: " + asset.path + " tags=" + std::to_string(asset.tagFrames.size()));
    }
}

} // namespace

void EngineBindings::init(Window* window, Renderer2D* renderer, Interpreter* interpreter) {
    resetBindingsState();
    st.window = window;
    st.renderer = renderer;
    st.interpreter = interpreter;
}

void EngineBindings::setAssetBase(const std::string& base) {
    st.assetBase = base;
    st.importPaths.clear();
    st.moduleDirStack.clear();
    if (!base.empty()) {
        st.importPaths.push_back(std::filesystem::path(base));
        st.moduleDirStack.push_back(std::filesystem::path(base));
    }
}

void EngineBindings::update(double dt) {
    hotReloadAse(dt);
    updateTweensTick(dt);
    cleanupTweens();
    updateAnimationsTick(dt);
}

void EngineBindings::registerBuiltins(std::unordered_map<std::string, NativeFn>& builtins) {
    registerCoreBuiltins(builtins);
    registerRenderBuiltins(builtins);
    registerAnimBuiltins(builtins);
    registerCollisionBuiltins(builtins);
    registerInputBuiltins(builtins);
    registerTweenBuiltins(builtins);
    registerMathBuiltins(builtins);
    registerAseBuiltins(builtins);
}
} // namespace yuki
