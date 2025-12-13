#pragma once
#include "../../script/value.hpp"
#include <vector>

namespace yuki {
Value apiSetClearColor(const std::vector<Value>& args);
Value apiDrawRect(const std::vector<Value>& args);
Value apiLoadSprite(const std::vector<Value>& args);
Value apiLoadSpriteSheet(const std::vector<Value>& args);
Value apiLoadFont(const std::vector<Value>& args);
Value apiDrawSprite(const std::vector<Value>& args);
Value apiDrawSpriteEx(const std::vector<Value>& args);
Value apiDrawSpriteFrame(const std::vector<Value>& args);
Value apiDrawText(const std::vector<Value>& args);
Value apiMeasureTextWidth(const std::vector<Value>& args);
Value apiMeasureTextHeight(const std::vector<Value>& args);
Value apiSetDebugDrawEnabled(const std::vector<Value>& args);
Value apiDebugDrawRect(const std::vector<Value>& args);
Value apiDebugDrawLine(const std::vector<Value>& args);
Value apiSetVirtualResolution(const std::vector<Value>& args);
Value apiCameraSet(const std::vector<Value>& args);
Value apiCameraSetZoom(const std::vector<Value>& args);
Value apiCameraSetRotation(const std::vector<Value>& args);
Value apiCameraFollowTarget(const std::vector<Value>& args);
Value apiCameraFollowEnable(const std::vector<Value>& args);
Value apiCameraFollowLerp(const std::vector<Value>& args);
} // namespace yuki
