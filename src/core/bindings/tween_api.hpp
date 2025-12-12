#pragma once
#include "../../script/value.hpp"
#include <vector>

namespace yuki {
Value apiTweenValue(const std::vector<Value>& args);
Value apiTweenValueGet(const std::vector<Value>& args);
Value apiTweenProperty(const std::vector<Value>& args);
Value apiTweenSequenceStart(const std::vector<Value>& args);
Value apiTweenSequenceAdd(const std::vector<Value>& args);
Value apiTweenSequencePlay(const std::vector<Value>& args);
Value apiTweenParallelStart(const std::vector<Value>& args);
Value apiTweenParallelAdd(const std::vector<Value>& args);
Value apiTweenParallelPlay(const std::vector<Value>& args);
Value apiTweenPause(const std::vector<Value>& args);
Value apiTweenResume(const std::vector<Value>& args);
Value apiTweenCancel(const std::vector<Value>& args);
Value apiTweenOnComplete(const std::vector<Value>& args);
void updateTweensTick(double dt);
void cleanupTweens();

// Animation helpers built on tweens
Value apiShake(const std::vector<Value>& args);
Value apiSquash(const std::vector<Value>& args);
Value apiBounce(const std::vector<Value>& args);
Value apiFlash(const std::vector<Value>& args);
} // namespace yuki
