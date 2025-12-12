#include "register_bindings.hpp"
#include "core_api.hpp"

namespace yuki {
void registerCoreBuiltins(std::unordered_map<std::string, NativeFn>& builtins) {
    builtins["engine_log"] = apiLog;
    builtins["import"] = apiImport;
    builtins["time"] = apiTime;
    builtins["random"] = apiRandom;
    builtins["get_screen_size"] = apiGetScreenSize;
}
} // namespace yuki
