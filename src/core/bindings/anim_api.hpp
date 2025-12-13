#pragma once
#include "../../script/value.hpp"
#include <vector>

namespace yuki {
Value apiAnimCreate(const std::vector<Value>& args);
Value apiAnimPlay(const std::vector<Value>& args);
Value apiAnimStop(const std::vector<Value>& args);
Value apiAnimReset(const std::vector<Value>& args);
Value apiAnimSetPosition(const std::vector<Value>& args);
Value apiAnimSetOrigin(const std::vector<Value>& args);
Value apiAnimSetScale(const std::vector<Value>& args);
Value apiAnimSetRotation(const std::vector<Value>& args);
Value apiAnimSetFlip(const std::vector<Value>& args);
Value apiAnimSetAlpha(const std::vector<Value>& args);
Value apiAnimDraw(const std::vector<Value>& args);
Value apiAnimGetPosition(const std::vector<Value>& args);
Value apiAnimGetScale(const std::vector<Value>& args);
Value apiAnimGetRotation(const std::vector<Value>& args);
Value apiAnimGetAlpha(const std::vector<Value>& args);

void updateAnimationsTick(double dt);
} // namespace yuki
