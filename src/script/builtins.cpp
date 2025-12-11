#include "builtins.hpp"
#include "value.hpp"
#include <iostream>
#include <unordered_map>

namespace yuki {

Value builtinPrint(const std::vector<Value>& args) {
    for (size_t i = 0; i < args.size(); ++i) {
        std::cout << args[i].toString();
        if (i < args.size() - 1) std::cout << " ";
    }
    std::cout << std::endl;
    return Value::nilVal();
}

Value builtinArray(const std::vector<Value>& args) {
    return Value::array(args);
}
Value builtinArrayPush(const std::vector<Value>& args) {
    if (args.empty() || !args[0].isArray() || !args[0].arrayPtr) return Value::nilVal();
    if (args.size() < 2) return Value::nilVal();
    args[0].arrayPtr->push_back(args[1]);
    return Value::nilVal();
}
Value builtinArrayPop(const std::vector<Value>& args) {
    if (args.empty() || !args[0].isArray() || !args[0].arrayPtr) return Value::nilVal();
    auto& arr = *args[0].arrayPtr;
    if (arr.empty()) return Value::nilVal();
    Value v = arr.back();
    arr.pop_back();
    return v;
}
Value builtinArrayLen(const std::vector<Value>& args) {
    if (args.empty() || !args[0].isArray() || !args[0].arrayPtr) return Value::number(0);
    return Value::number((double)args[0].arrayPtr->size());
}
Value builtinArrayGet(const std::vector<Value>& args) {
    if (args.size() < 2 || !args[0].isArray() || !args[0].arrayPtr || !args[1].isNumber()) return Value::nilVal();
    auto& arr = *args[0].arrayPtr;
    int idx = (int)args[1].numberVal;
    if (idx < 0 || idx >= (int)arr.size()) return Value::nilVal();
    return arr[idx];
}
Value builtinArraySet(const std::vector<Value>& args) {
    if (args.size() < 3 || !args[0].isArray() || !args[0].arrayPtr || !args[1].isNumber()) return Value::nilVal();
    auto& arr = *args[0].arrayPtr;
    int idx = (int)args[1].numberVal;
    if (idx < 0) return Value::nilVal();
    if (idx >= (int)arr.size()) {
        arr.resize(idx + 1, Value::nilVal());
    }
    arr[idx] = args[2];
    return Value::nilVal();
}
Value builtinMap(const std::vector<Value>& args) {
    std::unordered_map<std::string, Value> m;
    for (size_t i = 0; i + 1 < args.size(); i += 2) {
        m[args[i].toString()] = args[i + 1];
    }
    return Value::map(m);
}
Value builtinMapSet(const std::vector<Value>& args) {
    if (args.size() < 3 || !args[0].isMap() || !args[0].mapPtr) return Value::nilVal();
    (*args[0].mapPtr)[args[1].toString()] = args[2];
    return Value::nilVal();
}
Value builtinMapGet(const std::vector<Value>& args) {
    if (args.size() < 2 || !args[0].isMap() || !args[0].mapPtr) return Value::nilVal();
    auto it = args[0].mapPtr->find(args[1].toString());
    if (it == args[0].mapPtr->end()) return Value::nilVal();
    return it->second;
}
Value builtinMapHas(const std::vector<Value>& args) {
    if (args.size() < 2 || !args[0].isMap() || !args[0].mapPtr) return Value::boolean(false);
    auto it = args[0].mapPtr->find(args[1].toString());
    return Value::boolean(it != args[0].mapPtr->end());
}
Value builtinMapKeys(const std::vector<Value>& args) {
    if (args.size() < 1 || !args[0].isMap() || !args[0].mapPtr) return Value::array({});
    std::vector<Value> keys;
    keys.reserve(args[0].mapPtr->size());
    for (const auto& kv : *args[0].mapPtr) {
        keys.push_back(Value::string(kv.first));
    }
    return Value::array(keys);
}

void registerScriptBuiltins(std::unordered_map<std::string, NativeFn>& builtins) {
    builtins["print"] = builtinPrint;
    builtins["array"] = builtinArray;
    builtins["array_push"] = builtinArrayPush;
    builtins["array_pop"] = builtinArrayPop;
    builtins["array_len"] = builtinArrayLen;
    builtins["array_get"] = builtinArrayGet;
    builtins["array_set"] = builtinArraySet;
    builtins["map"] = builtinMap;
    builtins["map_set"] = builtinMapSet;
    builtins["map_get"] = builtinMapGet;
    builtins["map_has"] = builtinMapHas;
    builtins["map_keys"] = builtinMapKeys;
}

}
