#pragma once

#include <unordered_map>
#include <string>
#include "../script/function_value.hpp"

namespace yuki {
    void registerScriptBuiltins(std::unordered_map<std::string, NativeFn>& builtins);
    Value builtinPrint(const std::vector<Value>& args);
    Value builtinArray(const std::vector<Value>& args);
    Value builtinArrayPush(const std::vector<Value>& args);
    Value builtinArrayPop(const std::vector<Value>& args);
    Value builtinArrayLen(const std::vector<Value>& args);
    Value builtinArrayGet(const std::vector<Value>& args);
    Value builtinArraySet(const std::vector<Value>& args);
    Value builtinMap(const std::vector<Value>& args);
    Value builtinMapSet(const std::vector<Value>& args);
    Value builtinMapGet(const std::vector<Value>& args);
    Value builtinMapHas(const std::vector<Value>& args);
    Value builtinMapKeys(const std::vector<Value>& args);
    Value builtinStrLen(const std::vector<Value>& args);
    Value builtinStrLower(const std::vector<Value>& args);
    Value builtinStrUpper(const std::vector<Value>& args);
    Value builtinStrSub(const std::vector<Value>& args);
    Value builtinStrFind(const std::vector<Value>& args);
    Value builtinJoin(const std::vector<Value>& args);
}
