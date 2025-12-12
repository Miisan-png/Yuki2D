#include "state.hpp"

namespace yuki {
static BindingsState g_State;

BindingsState& bindingsState() {
    return g_State;
}

void resetBindingsState() {
    g_State = BindingsState{};
}
} // namespace yuki
