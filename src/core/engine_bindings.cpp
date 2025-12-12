#include "engine_bindings.hpp"
#include "bindings/state.hpp"
#include "bindings/register_bindings.hpp"
#include "bindings/anim_api.hpp"
#include "bindings/tween_api.hpp"
#include <filesystem>
#include <functional>

namespace yuki {
namespace {
BindingsState& st = bindingsState();

void debugDrawColliders() {
    if (!st.renderer || !st.renderer->isDebugEnabled()) return;
    for (const auto& c : st.colliders) {
        if (c.w <= 0.0f || c.h <= 0.0f) continue;
        std::hash<std::string> h;
        size_t hv = h(c.tag);
        float r = ((hv >> 0) & 0xFF) / 255.0f;
        float g = ((hv >> 8) & 0xFF) / 255.0f;
        float b = ((hv >> 16) & 0xFF) / 255.0f;
        st.renderer->debugDrawRect(c.x, c.y, c.w, c.h, r, g, b);
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
    if (!base.empty()) {
        st.importPaths.push_back(std::filesystem::path(base));
    }
}

void EngineBindings::update(double dt) {
    updateTweensTick(dt);
    cleanupTweens();
    updateAnimationsTick(dt);
    debugDrawColliders();
}

void EngineBindings::registerBuiltins(std::unordered_map<std::string, NativeFn>& builtins) {
    registerCoreBuiltins(builtins);
    registerRenderBuiltins(builtins);
    registerAnimBuiltins(builtins);
    registerCollisionBuiltins(builtins);
    registerInputBuiltins(builtins);
    registerTweenBuiltins(builtins);
    registerMathBuiltins(builtins);
}
} // namespace yuki
