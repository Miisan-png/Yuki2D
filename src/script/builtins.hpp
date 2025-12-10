#pragma once
#include <vector>
#include "value.hpp"
namespace yuki {
using NativeFn = Value(*)(const std::vector<Value>& args);
Value builtinPrint(const std::vector<Value>& args);
}
