#pragma once
#include "../../script/value.hpp"
#include <vector>

namespace yuki {
Value apiIsKeyDown(const std::vector<Value>& args);
Value apiIsKeyPressed(const std::vector<Value>& args);
Value apiIsKeyReleased(const std::vector<Value>& args);
Value apiIsMouseDown(const std::vector<Value>& args);
Value apiIsMousePressed(const std::vector<Value>& args);
Value apiIsMouseReleased(const std::vector<Value>& args);
Value apiGetMouseX(const std::vector<Value>& args);
Value apiGetMouseY(const std::vector<Value>& args);
Value apiBindAction(const std::vector<Value>& args);
Value apiUnbindAction(const std::vector<Value>& args);
Value apiActionDown(const std::vector<Value>& args);
Value apiActionPressed(const std::vector<Value>& args);
Value apiActionReleased(const std::vector<Value>& args);
} // namespace yuki
