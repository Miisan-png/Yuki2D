#include "register_bindings.hpp"
#include "anim_api.hpp"

namespace yuki {
void registerAnimBuiltins(std::unordered_map<std::string, NativeFn>& builtins) {
    builtins["anim_create"] = apiAnimCreate;
    builtins["anim_play"] = apiAnimPlay;
    builtins["anim_stop"] = apiAnimStop;
    builtins["anim_reset"] = apiAnimReset;
    builtins["anim_set_position"] = apiAnimSetPosition;
    builtins["anim_set_origin"] = apiAnimSetOrigin;
    builtins["anim_set_scale"] = apiAnimSetScale;
    builtins["anim_set_rotation"] = apiAnimSetRotation;
    builtins["anim_set_flip"] = apiAnimSetFlip;
    builtins["anim_set_alpha"] = apiAnimSetAlpha;
    builtins["anim_draw"] = apiAnimDraw;
}
} // namespace yuki
