#pragma once
#include "../../script/value.hpp"
#include <vector>

namespace yuki {
Value apiLog(const std::vector<Value>& args);
Value apiImport(const std::vector<Value>& args);
Value apiTime(const std::vector<Value>& args);
Value apiRandom(const std::vector<Value>& args);
Value apiGetScreenSize(const std::vector<Value>& args);
} // namespace yuki
