#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "../script/value.hpp"
#include "../script/function_value.hpp"
namespace yuki {
class Window;
class Renderer2D;
class Interpreter;
class EngineBindings {
public:
    static void init(Window* window, Renderer2D* renderer, class Interpreter* interpreter);
    static void update(double dt);
    static void setAssetBase(const std::string& base);
    static void registerBuiltins(std::unordered_map<std::string, NativeFn>& builtins);
};
Value engineLog(const std::vector<Value>& args);
Value engineTime(const std::vector<Value>& args);
Value engineRandom(const std::vector<Value>& args);
Value engineSetClearColor(const std::vector<Value>& args);
Value engineDrawRect(const std::vector<Value>& args);
Value engineLoadSprite(const std::vector<Value>& args);
Value engineLoadSpriteSheet(const std::vector<Value>& args);
Value engineDrawSprite(const std::vector<Value>& args);
Value engineDrawSpriteEx(const std::vector<Value>& args);
Value engineDrawSpriteFrame(const std::vector<Value>& args);
Value engineAnimCreate(const std::vector<Value>& args);
Value engineAnimPlay(const std::vector<Value>& args);
Value engineAnimStop(const std::vector<Value>& args);
Value engineAnimReset(const std::vector<Value>& args);
Value engineAnimSetPosition(const std::vector<Value>& args);
Value engineAnimSetOrigin(const std::vector<Value>& args);
Value engineAnimSetScale(const std::vector<Value>& args);
Value engineAnimSetRotation(const std::vector<Value>& args);
Value engineAnimSetFlip(const std::vector<Value>& args);
Value engineAnimSetAlpha(const std::vector<Value>& args);
Value engineAnimDraw(const std::vector<Value>& args);
Value engineColliderCreate(const std::vector<Value>& args);
Value engineColliderSetPos(const std::vector<Value>& args);
Value engineColliderSetSize(const std::vector<Value>& args);
Value engineColliderGetPos(const std::vector<Value>& args);
Value engineColliderGetSize(const std::vector<Value>& args);
Value engineColliderMove(const std::vector<Value>& args);
Value engineSetDebugDrawEnabled(const std::vector<Value>& args);
Value engineDebugDrawRect(const std::vector<Value>& args);
Value engineDebugDrawLine(const std::vector<Value>& args);
Value engineSin(const std::vector<Value>& args);
Value engineCos(const std::vector<Value>& args);
Value engineIsKeyDown(const std::vector<Value>& args);
Value engineIsKeyPressed(const std::vector<Value>& args);
Value engineIsKeyReleased(const std::vector<Value>& args);
Value engineTweenValue(const std::vector<Value>& args);
Value engineTweenValueGet(const std::vector<Value>& args);
Value engineTweenProperty(const std::vector<Value>& args);
Value engineTweenSequenceStart(const std::vector<Value>& args);
Value engineTweenSequenceAdd(const std::vector<Value>& args);
Value engineTweenSequencePlay(const std::vector<Value>& args);
Value engineTweenParallelStart(const std::vector<Value>& args);
Value engineTweenParallelAdd(const std::vector<Value>& args);
Value engineTweenParallelPlay(const std::vector<Value>& args);
Value engineTweenPause(const std::vector<Value>& args);
Value engineTweenResume(const std::vector<Value>& args);
Value engineTweenCancel(const std::vector<Value>& args);
Value engineTweenOnComplete(const std::vector<Value>& args);
Value engineShake(const std::vector<Value>& args);
Value engineSquash(const std::vector<Value>& args);
Value engineBounce(const std::vector<Value>& args);
Value engineFlash(const std::vector<Value>& args);
Value engineRectOverlaps(const std::vector<Value>& args);
Value enginePointInRect(const std::vector<Value>& args);
// get_screen_size registered inline in cpp
Value engineCreateAreaRect(const std::vector<Value>& args);
Value engineSetAreaRect(const std::vector<Value>& args);
Value engineAreaOverlaps(const std::vector<Value>& args);
Value engineAreaOverlapsTag(const std::vector<Value>& args);
Value engineAreaEnteredTag(const std::vector<Value>& args);
Value engineAreaExitedTag(const std::vector<Value>& args);
Value engineDebugArea(const std::vector<Value>& args);
}
