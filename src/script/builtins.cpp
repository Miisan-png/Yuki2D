#include "builtins.hpp"
#include "value.hpp"
#include <iostream>
#include <unordered_map>
#include <cmath>

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
Value builtinArraySlice(const std::vector<Value>& args) {
    if (args.size() < 2 || !args[0].isArray() || !args[0].arrayPtr || !args[1].isNumber()) return Value::array({});
    int start = (int)args[1].numberVal;
    int len = (args.size() > 2 && args[2].isNumber()) ? (int)args[2].numberVal : (int)args[0].arrayPtr->size();
    if (start < 0) start = 0;
    if (len < 0) len = 0;
    int end = std::min((int)args[0].arrayPtr->size(), start + len);
    std::vector<Value> out;
    for (int i = start; i < end; ++i) out.push_back((*args[0].arrayPtr)[i]);
    return Value::array(out);
}
Value builtinArrayConcat(const std::vector<Value>& args) {
    if (args.size() < 2 || !args[0].isArray() || !args[0].arrayPtr || !args[1].isArray() || !args[1].arrayPtr) return Value::array({});
    std::vector<Value> out = *args[0].arrayPtr;
    out.insert(out.end(), args[1].arrayPtr->begin(), args[1].arrayPtr->end());
    return Value::array(out);
}
Value builtinArrayClear(const std::vector<Value>& args) {
    if (args.empty() || !args[0].isArray() || !args[0].arrayPtr) return Value::nilVal();
    args[0].arrayPtr->clear();
    return Value::nilVal();
}
Value builtinArrayIndexOf(const std::vector<Value>& args) {
    if (args.size() < 2 || !args[0].isArray() || !args[0].arrayPtr) return Value::number(-1);
    const auto& arr = *args[0].arrayPtr;
    const Value& needle = args[1];
    for (size_t i = 0; i < arr.size(); ++i) {
        if (arr[i].type == needle.type) {
            if (arr[i].isNumber() && needle.isNumber() && arr[i].numberVal == needle.numberVal) return Value::number((double)i);
            if (arr[i].isBool() && needle.isBool() && arr[i].boolVal == needle.boolVal) return Value::number((double)i);
            if (arr[i].isString() && needle.isString() && arr[i].stringVal == needle.stringVal) return Value::number((double)i);
            if (arr[i].isNil() && needle.isNil()) return Value::number((double)i);
        }
    }
    return Value::number(-1);
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
Value builtinMapValues(const std::vector<Value>& args) {
    if (args.size() < 1 || !args[0].isMap() || !args[0].mapPtr) return Value::array({});
    std::vector<Value> vals;
    vals.reserve(args[0].mapPtr->size());
    for (const auto& kv : *args[0].mapPtr) vals.push_back(kv.second);
    return Value::array(vals);
}
Value builtinMapDelete(const std::vector<Value>& args) {
    if (args.size() < 2 || !args[0].isMap() || !args[0].mapPtr) return Value::boolean(false);
    auto it = args[0].mapPtr->find(args[1].toString());
    if (it == args[0].mapPtr->end()) return Value::boolean(false);
    args[0].mapPtr->erase(it);
    return Value::boolean(true);
}
Value builtinMapMerge(const std::vector<Value>& args) {
    if (args.size() < 2 || !args[0].isMap() || !args[0].mapPtr || !args[1].isMap() || !args[1].mapPtr) return Value::map({});
    std::unordered_map<std::string, Value> m = *args[0].mapPtr;
    for (const auto& kv : *args[1].mapPtr) {
        m[kv.first] = kv.second;
    }
    return Value::map(m);
}
Value builtinMapSize(const std::vector<Value>& args) {
    if (args.empty() || !args[0].isMap() || !args[0].mapPtr) return Value::number(0);
    return Value::number((double)args[0].mapPtr->size());
}
Value builtinStrLen(const std::vector<Value>& args) {
    if (args.empty() || !args[0].isString()) return Value::number(0);
    return Value::number((double)args[0].stringVal.size());
}
Value builtinStrLower(const std::vector<Value>& args) {
    if (args.empty() || !args[0].isString()) return Value::string("");
    std::string s = args[0].stringVal;
    for (char& c : s) c = (char)std::tolower((unsigned char)c);
    return Value::string(s);
}
Value builtinStrUpper(const std::vector<Value>& args) {
    if (args.empty() || !args[0].isString()) return Value::string("");
    std::string s = args[0].stringVal;
    for (char& c : s) c = (char)std::toupper((unsigned char)c);
    return Value::string(s);
}
Value builtinStrSub(const std::vector<Value>& args) {
    if (args.size() < 2 || !args[0].isString() || !args[1].isNumber()) return Value::string("");
    std::string s = args[0].stringVal;
    int start = (int)args[1].numberVal;
    int len = (args.size() > 2 && args[2].isNumber()) ? (int)args[2].numberVal : (int)s.size();
    if (start < 0) start = 0;
    if (len < 0) len = 0;
    if (start >= (int)s.size()) return Value::string("");
    int end = std::min((int)s.size(), start + len);
    return Value::string(s.substr(start, end - start));
}
Value builtinStrFind(const std::vector<Value>& args) {
    if (args.size() < 2 || !args[0].isString() || !args[1].isString()) return Value::number(-1);
    size_t pos = args[0].stringVal.find(args[1].stringVal);
    if (pos == std::string::npos) return Value::number(-1);
    return Value::number((double)pos);
}
Value builtinJoin(const std::vector<Value>& args) {
    if (args.size() < 2 || !args[0].isArray() || !args[0].arrayPtr || !args[1].isString()) return Value::string("");
    const auto& arr = *args[0].arrayPtr;
    std::string sep = args[1].stringVal;
    std::string out;
    for (size_t i = 0; i < arr.size(); ++i) {
        out += arr[i].toString();
        if (i + 1 < arr.size()) out += sep;
    }
    return Value::string(out);
}
Value builtinStrReplace(const std::vector<Value>& args) {
    if (args.size() < 3 || !args[0].isString() || !args[1].isString() || !args[2].isString()) return Value::string("");
    std::string s = args[0].stringVal;
    const std::string& from = args[1].stringVal;
    const std::string& to = args[2].stringVal;
    if (from.empty()) return Value::string(s);
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
    return Value::string(s);
}
Value builtinStrSplit(const std::vector<Value>& args) {
    if (args.size() < 2 || !args[0].isString() || !args[1].isString()) return Value::array({});
    std::string s = args[0].stringVal;
    std::string delim = args[1].stringVal;
    if (delim.empty()) return Value::array({Value::string(s)});
    std::vector<Value> parts;
    size_t start = 0;
    size_t pos = s.find(delim);
    while (pos != std::string::npos) {
        parts.push_back(Value::string(s.substr(start, pos - start)));
        start = pos + delim.size();
        pos = s.find(delim, start);
    }
    parts.push_back(Value::string(s.substr(start)));
    return Value::array(parts);
}

void registerScriptBuiltins(std::unordered_map<std::string, NativeFn>& builtins) {
    builtins["print"] = builtinPrint;
    builtins["array"] = builtinArray;
    builtins["array_push"] = builtinArrayPush;
    builtins["array_pop"] = builtinArrayPop;
    builtins["array_len"] = builtinArrayLen;
    builtins["array_get"] = builtinArrayGet;
    builtins["array_set"] = builtinArraySet;
    builtins["array_slice"] = builtinArraySlice;
    builtins["array_concat"] = builtinArrayConcat;
    builtins["array_clear"] = builtinArrayClear;
    builtins["array_index_of"] = builtinArrayIndexOf;
    builtins["map"] = builtinMap;
    builtins["map_set"] = builtinMapSet;
    builtins["map_get"] = builtinMapGet;
    builtins["map_has"] = builtinMapHas;
    builtins["map_keys"] = builtinMapKeys;
    builtins["map_values"] = builtinMapValues;
    builtins["map_delete"] = builtinMapDelete;
    builtins["map_merge"] = builtinMapMerge;
    builtins["map_size"] = builtinMapSize;
    builtins["str_len"] = builtinStrLen;
    builtins["str_lower"] = builtinStrLower;
    builtins["str_upper"] = builtinStrUpper;
    builtins["str_sub"] = builtinStrSub;
    builtins["str_find"] = builtinStrFind;
    builtins["join"] = builtinJoin;
    builtins["str_replace"] = builtinStrReplace;
    builtins["str_split"] = builtinStrSplit;
    builtins["sqrt"] = [](const std::vector<Value>& args) -> Value {
        if (args.size() >= 1 && args[0].isNumber()) return Value::number(std::sqrt(args[0].numberVal));
        return Value::number(0);
    };
    builtins["sin"] = [](const std::vector<Value>& args) -> Value {
        if (args.size() >= 1 && args[0].isNumber()) return Value::number(std::sin(args[0].numberVal));
        return Value::number(0);
    };
    builtins["cos"] = [](const std::vector<Value>& args) -> Value {
        if (args.size() >= 1 && args[0].isNumber()) return Value::number(std::cos(args[0].numberVal));
        return Value::number(0);
    };
    builtins["abs"] = [](const std::vector<Value>& args) -> Value {
        if (args.size() >= 1 && args[0].isNumber()) return Value::number(std::fabs(args[0].numberVal));
        return Value::number(0);
    };
    builtins["floor"] = [](const std::vector<Value>& args) -> Value {
        if (args.size() >= 1 && args[0].isNumber()) return Value::number(std::floor(args[0].numberVal));
        return Value::number(0);
    };
    builtins["ceil"] = [](const std::vector<Value>& args) -> Value {
        if (args.size() >= 1 && args[0].isNumber()) return Value::number(std::ceil(args[0].numberVal));
        return Value::number(0);
    };
}

}
