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
Value engineDrawSprite(const std::vector<Value>& args);
Value engineDrawSpriteEx(const std::vector<Value>& args);
Value engineSetCameraPos(const std::vector<Value>& args);
Value engineSetCameraZoom(const std::vector<Value>& args);
Value engineGetCameraPosX(const std::vector<Value>& args);
Value engineGetCameraPosY(const std::vector<Value>& args);
Value engineGetCameraZoom(const std::vector<Value>& args);
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
Value engineCreateAreaRect(const std::vector<Value>& args);
Value engineSetAreaRect(const std::vector<Value>& args);
Value engineAreaOverlaps(const std::vector<Value>& args);
Value engineAreaOverlapsTag(const std::vector<Value>& args);
Value engineAreaEnteredTag(const std::vector<Value>& args);
Value engineAreaExitedTag(const std::vector<Value>& args);
Value engineDebugArea(const std::vector<Value>& args);
}
