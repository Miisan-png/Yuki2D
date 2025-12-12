#include "register_bindings.hpp"
#include "render_api.hpp"

namespace yuki {
void registerRenderBuiltins(std::unordered_map<std::string, NativeFn>& builtins) {
    builtins["set_clear_color"] = apiSetClearColor;
    builtins["draw_rect"] = apiDrawRect;
    builtins["load_sprite"] = apiLoadSprite;
    builtins["load_sprite_sheet"] = apiLoadSpriteSheet;
    builtins["draw_sprite"] = apiDrawSprite;
    builtins["draw_sprite_ex"] = apiDrawSpriteEx;
    builtins["draw_sprite_frame"] = apiDrawSpriteFrame;
    builtins["load_font"] = apiLoadFont;
    builtins["draw_text"] = apiDrawText;
    builtins["draw_text_ex"] = apiDrawText;
    builtins["measure_text_width"] = apiMeasureTextWidth;
    builtins["measure_text_height"] = apiMeasureTextHeight;
    builtins["set_debug_draw_enabled"] = apiSetDebugDrawEnabled;
    builtins["debug_draw_rect"] = apiDebugDrawRect;
    builtins["debug_draw_line"] = apiDebugDrawLine;
    builtins["set_virtual_resolution"] = apiSetVirtualResolution;
}
} // namespace yuki
