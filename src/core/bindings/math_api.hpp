#pragma once
#include "../../script/value.hpp"
#include <vector>

namespace yuki {
Value apiSin(const std::vector<Value>& args);
Value apiCos(const std::vector<Value>& args);
} // namespace yuki
