#include "register_bindings.hpp"
#include "tween_api.hpp"

namespace yuki {
void registerTweenBuiltins(std::unordered_map<std::string, NativeFn>& builtins) {
    builtins["tween_value"] = apiTweenValue;
    builtins["tween_value_get"] = apiTweenValueGet;
    builtins["tween_property"] = apiTweenProperty;
    builtins["tween_sequence_start"] = apiTweenSequenceStart;
    builtins["tween_sequence_add"] = apiTweenSequenceAdd;
    builtins["tween_sequence_play"] = apiTweenSequencePlay;
    builtins["tween_parallel_start"] = apiTweenParallelStart;
    builtins["tween_parallel_add"] = apiTweenParallelAdd;
    builtins["tween_parallel_play"] = apiTweenParallelPlay;
    builtins["tween_pause"] = apiTweenPause;
    builtins["tween_resume"] = apiTweenResume;
    builtins["tween_cancel"] = apiTweenCancel;
    builtins["tween_on_complete"] = apiTweenOnComplete;
    builtins["shake"] = apiShake;
    builtins["squash"] = apiSquash;
    builtins["bounce"] = apiBounce;
    builtins["flash"] = apiFlash;
}
} // namespace yuki
