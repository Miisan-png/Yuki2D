#include "register_bindings.hpp"
#include "collision_api.hpp"

namespace yuki {
void registerCollisionBuiltins(std::unordered_map<std::string, NativeFn>& builtins) {
    builtins["collider_create"] = apiColliderCreate;
    builtins["collider_set_position"] = apiColliderSetPos;
    builtins["collider_set_size"] = apiColliderSetSize;
    builtins["collider_get_position"] = apiColliderGetPos;
    builtins["collider_get_size"] = apiColliderGetSize;
    builtins["collider_move"] = apiColliderMove;
    builtins["rect_overlaps"] = apiRectOverlaps;
    builtins["point_in_rect"] = apiPointInRect;
    builtins["create_area_rect"] = apiCreateAreaRect;
    builtins["set_area_rect"] = apiSetAreaRect;
    builtins["area_overlaps"] = apiAreaOverlaps;
    builtins["area_overlaps_tag"] = apiAreaOverlapsTag;
    builtins["area_entered_tag"] = apiAreaEnteredTag;
    builtins["area_exited_tag"] = apiAreaExitedTag;
    builtins["debug_area"] = apiDebugArea;
}
} // namespace yuki
