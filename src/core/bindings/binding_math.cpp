#include "register_bindings.hpp"
#include "math_api.hpp"

namespace yuki {
void registerMathBuiltins(std::unordered_map<std::string, NativeFn>& builtins) {
    builtins["sin"] = apiSin;
    builtins["cos"] = apiCos;
}
} // namespace yuki
