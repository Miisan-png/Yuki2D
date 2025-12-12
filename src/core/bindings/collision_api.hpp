#pragma once
#include "../../script/value.hpp"
#include <vector>

namespace yuki {
Value apiColliderCreate(const std::vector<Value>& args);
Value apiColliderSetPos(const std::vector<Value>& args);
Value apiColliderSetSize(const std::vector<Value>& args);
Value apiColliderGetPos(const std::vector<Value>& args);
Value apiColliderGetSize(const std::vector<Value>& args);
Value apiColliderMove(const std::vector<Value>& args);

Value apiRectOverlaps(const std::vector<Value>& args);
Value apiPointInRect(const std::vector<Value>& args);

Value apiCreateAreaRect(const std::vector<Value>& args);
Value apiSetAreaRect(const std::vector<Value>& args);
Value apiAreaOverlaps(const std::vector<Value>& args);
Value apiAreaOverlapsTag(const std::vector<Value>& args);
Value apiAreaEnteredTag(const std::vector<Value>& args);
Value apiAreaExitedTag(const std::vector<Value>& args);
Value apiDebugArea(const std::vector<Value>& args);
} // namespace yuki
