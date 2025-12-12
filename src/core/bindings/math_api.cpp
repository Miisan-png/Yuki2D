#include "math_api.hpp"
#include <cmath>

namespace yuki {
Value apiSin(const std::vector<Value>& args) {
    if (args.empty() || !args[0].isNumber()) return Value::number(0.0);
    return Value::number(std::sin(args[0].numberVal));
}
Value apiCos(const std::vector<Value>& args) {
    if (args.empty() || !args[0].isNumber()) return Value::number(0.0);
    return Value::number(std::cos(args[0].numberVal));
}
} // namespace yuki
