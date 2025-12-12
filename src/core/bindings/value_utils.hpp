#pragma once
#include "../../script/value.hpp"
#include <cctype>
#include <string>

namespace yuki {
inline bool valueToBool(const Value& v, bool defaultVal = false) {
    if (v.isBool()) return v.boolVal;
    if (v.isNumber()) return v.numberVal != 0.0;
    if (v.isString()) {
        std::string s = v.stringVal;
        for (char& c : s) c = (char)std::tolower((unsigned char)c);
        if (s.empty() || s == "false" || s == "0") return false;
        return true;
    }
    return defaultVal;
}
}
