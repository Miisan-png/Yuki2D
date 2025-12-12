#include "register_bindings.hpp"
#include "input_api.hpp"

namespace yuki {
void registerInputBuiltins(std::unordered_map<std::string, NativeFn>& builtins) {
    builtins["is_key_down"] = apiIsKeyDown;
    builtins["is_key_pressed"] = apiIsKeyPressed;
    builtins["is_key_released"] = apiIsKeyReleased;
    builtins["is_mouse_down"] = apiIsMouseDown;
    builtins["is_mouse_pressed"] = apiIsMousePressed;
    builtins["is_mouse_released"] = apiIsMouseReleased;
    builtins["get_mouse_x"] = apiGetMouseX;
    builtins["get_mouse_y"] = apiGetMouseY;
    builtins["bind_action"] = apiBindAction;
    builtins["unbind_action"] = apiUnbindAction;
    builtins["action_down"] = apiActionDown;
    builtins["action_pressed"] = apiActionPressed;
    builtins["action_released"] = apiActionReleased;
}
} // namespace yuki
