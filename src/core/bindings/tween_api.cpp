#include "tween_api.hpp"
#include "state.hpp"
#include "../renderer2d.hpp"
#include "../../script/interpreter.hpp"

namespace yuki {
namespace {
BindingsState& st = bindingsState();

double clamp01(double v) {
    if (v < 0.0) return 0.0;
    if (v > 1.0) return 1.0;
    return v;
}
double applyEasing(double t, const std::string& easing) {
    double u = clamp01(t);
    if (easing == "ease_in") return u * u;
    if (easing == "ease_out") return u * (2.0 - u);
    if (easing == "ease_in_out") {
        if (u < 0.5) return 2.0 * u * u;
        double k = -2.0 * u + 2.0;
        return 1.0 - (k * k) / 2.0;
    }
    return u;
}

bool getPropertyValue(const TweenTarget& target, const std::string& prop, float& out) {
    if (target.type == TweenTargetType::Sprite) {
        auto it = st.spriteStates.find(target.id);
        if (it == st.spriteStates.end()) return false;
        const auto& s = it->second;
        if (prop == "x") { out = s.x; return true; }
        if (prop == "y") { out = s.y; return true; }
        if (prop == "rotation") { out = s.rotation; return true; }
        if (prop == "scale_x") { out = s.scaleX; return true; }
        if (prop == "scale_y") { out = s.scaleY; return true; }
        if (prop == "alpha") { out = s.alpha; return true; }
    }
    if (target.type == TweenTargetType::Animation) {
        auto it = st.animations.find(target.id);
        if (it == st.animations.end()) return false;
        const auto& a = it->second;
        if (prop == "x") { out = a.transform.x; return true; }
        if (prop == "y") { out = a.transform.y; return true; }
        if (prop == "rotation") { out = a.transform.rotationDeg; return true; }
        if (prop == "scale_x") { out = a.transform.scaleX; return true; }
        if (prop == "scale_y") { out = a.transform.scaleY; return true; }
        if (prop == "alpha") { out = a.alpha; return true; }
    }
    return false;
}
void setPropertyValue(const TweenTarget& target, const std::string& prop, float v) {
    if (target.type == TweenTargetType::Sprite) {
        auto& s = st.spriteStates[target.id];
        if (prop == "x") { s.x = v; s.overrideX = true; }
        else if (prop == "y") { s.y = v; s.overrideY = true; }
        else if (prop == "rotation") { s.rotation = v; s.overrideRot = true; }
        else if (prop == "scale_x") { s.scaleX = v; s.overrideScaleX = true; }
        else if (prop == "scale_y") { s.scaleY = v; s.overrideScaleY = true; }
        else if (prop == "alpha") { s.alpha = v; s.overrideAlpha = true; }
    }
    if (target.type == TweenTargetType::Animation) {
        auto it = st.animations.find(target.id);
        if (it == st.animations.end()) return;
        auto& a = it->second;
        if (prop == "x") a.transform.x = v;
        else if (prop == "y") a.transform.y = v;
        else if (prop == "rotation") a.transform.rotationDeg = v;
        else if (prop == "scale_x") a.transform.scaleX = v;
        else if (prop == "scale_y") a.transform.scaleY = v;
        else if (prop == "alpha") a.alpha = v;
    }
}
TweenTarget parseTarget(const Value& obj) {
    TweenTarget tgt;
    int id = -1;
    if (obj.isNumber()) id = (int)obj.numberVal;
    else if (obj.isMap() && obj.mapPtr) {
        auto it = obj.mapPtr->find("id");
        if (it != obj.mapPtr->end() && it->second.isNumber()) id = (int)it->second.numberVal;
    }
    if (id >= 0) {
        if (st.animations.find(id) != st.animations.end()) {
            tgt.type = TweenTargetType::Animation;
            tgt.id = id;
        } else {
            tgt.type = TweenTargetType::Sprite;
            tgt.id = id;
        }
    }
    return tgt;
}
int nextTweenId() { return st.tweenCounter++; }
int nextSequenceId() { return st.sequenceCounter++; }
int nextParallelId() { return st.parallelCounter++; }
void resetTween(Tween& t, bool pause, bool refreshProperty) {
    if (refreshProperty && t.type == TweenType::Property) {
        float cur = (float)t.from;
        if (getPropertyValue(t.target, t.property, cur)) t.from = cur;
    }
    t.elapsed = 0.0;
    t.current = t.from;
    t.active = true;
    t.canceled = false;
    t.paused = pause;
}
void finishTween(Tween& t) {
    t.active = false;
    t.paused = false;
}
void invokeTweenCallback(Tween& t) {
    if (!st.interpreter) return;
    if (t.onComplete.isFunction()) {
        std::vector<Value> args;
        st.interpreter->callFunction(t.onComplete, args);
    }
}
int createValueTween(double from, double to, double duration, const std::string& easing) {
    int id = nextTweenId();
    Tween t;
    t.id = id;
    t.type = TweenType::Value;
    t.from = from;
    t.to = to;
    t.duration = duration;
    t.easing = easing;
    resetTween(t, false, false);
    st.tweens[id] = t;
    return id;
}
int createPropertyTween(const TweenTarget& target, const std::string& prop, double to, double duration, const std::string& easing) {
    int id = nextTweenId();
    Tween t;
    t.id = id;
    t.type = TweenType::Property;
    t.from = 0.0;
    t.to = to;
    t.duration = duration;
    t.easing = easing;
    t.target = target;
    t.property = prop;
    float cur = 0.0f;
    if (getPropertyValue(target, prop, cur)) t.from = cur;
    resetTween(t, false, false);
    st.tweens[id] = t;
    return id;
}
void updateTween(Tween& t, double dt) {
    if (t.canceled || t.paused || !t.active) return;
    t.elapsed += dt;
    double raw = t.elapsed / t.duration;
    double u = applyEasing(raw, t.easing);
    t.current = t.from + (t.to - t.from) * u;
    if (t.type == TweenType::Property) {
        setPropertyValue(t.target, t.property, (float)t.current);
    }
    if (t.elapsed >= t.duration) {
        t.current = t.to;
        if (t.type == TweenType::Property) {
            setPropertyValue(t.target, t.property, (float)t.current);
        }
        finishTween(t);
        invokeTweenCallback(t);
    }
}
} // namespace

// Public API wrappers
Value apiTweenValue(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::number(-1);
    double from = args[0].numberVal;
    double to = args[1].numberVal;
    double dur = args[2].numberVal;
    std::string easing = "linear";
    if (args.size() >= 4 && args[3].isString()) easing = args[3].stringVal;
    int id = createValueTween(from, to, dur, easing);
    return Value::number(id);
}
Value apiTweenValueGet(const std::vector<Value>& args) {
    if (args.empty()) return Value::number(0);
    int id = (int)args[0].numberVal;
    auto it = st.tweens.find(id);
    if (it == st.tweens.end()) return Value::number(0);
    return Value::number(it->second.current);
}
Value apiTweenProperty(const std::vector<Value>& args) {
    if (args.size() < 4) return Value::number(-1);
    TweenTarget tgt = parseTarget(args[0]);
    std::string prop = args[1].toString();
    double to = args[2].numberVal;
    double duration = args[3].numberVal;
    std::string easing = "linear";
    if (args.size() >= 5 && args[4].isString()) easing = args[4].stringVal;
    int id = createPropertyTween(tgt, prop, to, duration, easing);
    return Value::number(id);
}
Value apiTweenSequenceStart(const std::vector<Value>&) {
    int id = nextSequenceId();
    Sequence seq;
    seq.id = id;
    st.sequences[id] = seq;
    return Value::number(id);
}
Value apiTweenSequenceAdd(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::nilVal();
    int seqId = (int)args[0].numberVal;
    int tweenId = (int)args[1].numberVal;
    auto it = st.sequences.find(seqId);
    auto tw = st.tweens.find(tweenId);
    if (it == st.sequences.end() || tw == st.tweens.end()) return Value::nilVal();
    it->second.tweens.push_back(tweenId);
    st.tweenSequenceOwner[tweenId] = seqId;
    resetTween(tw->second, true, true);
    return Value::nilVal();
}
Value apiTweenSequencePlay(const std::vector<Value>& args) {
    if (args.empty()) return Value::nilVal();
    int seqId = (int)args[0].numberVal;
    auto it = st.sequences.find(seqId);
    if (it == st.sequences.end()) return Value::nilVal();
    it->second.playing = true;
    it->second.currentIndex = 0;
    if (!it->second.tweens.empty()) {
        int tid = it->second.tweens[0];
        auto tw = st.tweens.find(tid);
        if (tw != st.tweens.end()) resetTween(tw->second, false, true);
    }
    return Value::nilVal();
}
Value apiTweenParallelStart(const std::vector<Value>&) {
    int id = nextParallelId();
    ParallelGroup grp;
    grp.id = id;
    st.parallels[id] = grp;
    return Value::number(id);
}
Value apiTweenParallelAdd(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::nilVal();
    int pid = (int)args[0].numberVal;
    int tid = (int)args[1].numberVal;
    auto it = st.parallels.find(pid);
    auto tw = st.tweens.find(tid);
    if (it == st.parallels.end() || tw == st.tweens.end()) return Value::nilVal();
    it->second.tweens.push_back(tid);
    st.tweenParallelOwner[tid] = pid;
    resetTween(tw->second, true, true);
    return Value::nilVal();
}
Value apiTweenParallelPlay(const std::vector<Value>& args) {
    if (args.empty()) return Value::nilVal();
    int pid = (int)args[0].numberVal;
    auto it = st.parallels.find(pid);
    if (it == st.parallels.end()) return Value::nilVal();
    it->second.playing = true;
    for (int tid : it->second.tweens) {
        auto tw = st.tweens.find(tid);
        if (tw != st.tweens.end()) resetTween(tw->second, false, true);
    }
    return Value::nilVal();
}
Value apiTweenPause(const std::vector<Value>& args) {
    if (args.empty()) return Value::nilVal();
    int id = (int)args[0].numberVal;
    auto it = st.tweens.find(id);
    if (it != st.tweens.end()) it->second.paused = true;
    return Value::nilVal();
}
Value apiTweenResume(const std::vector<Value>& args) {
    if (args.empty()) return Value::nilVal();
    int id = (int)args[0].numberVal;
    auto it = st.tweens.find(id);
    if (it != st.tweens.end()) it->second.paused = false;
    return Value::nilVal();
}
Value apiTweenCancel(const std::vector<Value>& args) {
    if (args.empty()) return Value::nilVal();
    int id = (int)args[0].numberVal;
    auto it = st.tweens.find(id);
    if (it != st.tweens.end()) {
        it->second.canceled = true;
        finishTween(it->second);
    }
    return Value::nilVal();
}
Value apiTweenOnComplete(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::nilVal();
    int id = (int)args[0].numberVal;
    auto it = st.tweens.find(id);
    if (it != st.tweens.end() && args[1].isFunction()) {
        it->second.onComplete = args[1];
    }
    return Value::nilVal();
}

// Animation helpers built on tweens
Value apiShake(const std::vector<Value>& args) {
    if (args.size() < 4) return Value::number(-1);
    TweenTarget tgt = parseTarget(args[0]);
    double intensity = args[1].numberVal;
    double duration = args[2].numberVal;
    int freq = (int)args[3].numberVal;
    float baseX = 0.0f;
    float baseY = 0.0f;
    getPropertyValue(tgt, "x", baseX);
    getPropertyValue(tgt, "y", baseY);
    int seqId = nextSequenceId();
    Sequence s;
    s.id = seqId;
    double step = duration / (double)freq;
    for (int i = 0; i < freq; ++i) {
        double ox = ((i % 2 == 0) ? intensity : -intensity);
        double oy = ((i % 2 == 0) ? -intensity : intensity);
        int tx = createPropertyTween(tgt, "x", baseX + ox, step * 0.5, "linear");
        int ty = createPropertyTween(tgt, "y", baseY + oy, step * 0.5, "linear");
        s.tweens.push_back(tx);
        s.tweens.push_back(ty);
        st.tweenSequenceOwner[tx] = seqId;
        st.tweenSequenceOwner[ty] = seqId;
    }
    s.playing = true;
    st.sequences[seqId] = s;
    return Value::number(seqId);
}
Value apiSquash(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::number(-1);
    TweenTarget tgt = parseTarget(args[0]);
    double amount = args[1].numberVal;
    double duration = args[2].numberVal;
    float baseX = 1.0f;
    float baseY = 1.0f;
    getPropertyValue(tgt, "scale_x", baseX);
    getPropertyValue(tgt, "scale_y", baseY);
    int seqId = nextSequenceId();
    Sequence s;
    s.id = seqId;
    double half = duration * 0.5;
    int downX = createPropertyTween(tgt, "scale_x", baseX + amount, half, "ease_out");
    int downY = createPropertyTween(tgt, "scale_y", baseY - amount, half, "ease_out");
    int upX = createPropertyTween(tgt, "scale_x", baseX, half, "ease_in");
    int upY = createPropertyTween(tgt, "scale_y", baseY, half, "ease_in");
    s.tweens = {downX, downY, upX, upY};
    for (int tid : s.tweens) st.tweenSequenceOwner[tid] = seqId;
    s.playing = true;
    st.sequences[seqId] = s;
    return Value::number(seqId);
}
Value apiBounce(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::number(-1);
    TweenTarget tgt = parseTarget(args[0]);
    double height = args[1].numberVal;
    double duration = args[2].numberVal;
    float baseY = 0.0f;
    getPropertyValue(tgt, "y", baseY);
    int seqId = nextSequenceId();
    Sequence s;
    s.id = seqId;
    double half = duration * 0.5;
    int up = createPropertyTween(tgt, "y", baseY - height, half, "ease_out");
    int down = createPropertyTween(tgt, "y", baseY, half, "ease_in");
    s.tweens = {up, down};
    for (int tid : s.tweens) st.tweenSequenceOwner[tid] = seqId;
    s.playing = true;
    st.sequences[seqId] = s;
    return Value::number(seqId);
}
Value apiFlash(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::number(-1);
    TweenTarget tgt = parseTarget(args[0]);
    int times = (int)args[1].numberVal;
    double duration = args[2].numberVal;
    float baseA = 1.0f;
    getPropertyValue(tgt, "alpha", baseA);
    int seqId = nextSequenceId();
    Sequence s;
    s.id = seqId;
    double step = duration / (times * 2);
    for (int i = 0; i < times; ++i) {
        int off = createPropertyTween(tgt, "alpha", 0.0, step, "linear");
        int on = createPropertyTween(tgt, "alpha", baseA, step, "linear");
        s.tweens.push_back(off);
        s.tweens.push_back(on);
        st.tweenSequenceOwner[off] = seqId;
        st.tweenSequenceOwner[on] = seqId;
    }
    s.playing = true;
    st.sequences[seqId] = s;
    return Value::number(seqId);
}

// Update hooks
void updateTweens(double dt) {
    for (auto it = st.tweens.begin(); it != st.tweens.end();) {
        Tween& t = it->second;
        if (t.canceled) {
            it = st.tweens.erase(it);
            continue;
        }
        updateTween(t, dt);
        if (!t.active && !t.paused) it = st.tweens.erase(it);
        else ++it;
    }
    for (auto& kv : st.sequences) {
        Sequence& s = kv.second;
        if (!s.playing || s.finished) continue;
        if (s.currentIndex >= (int)s.tweens.size()) {
            s.finished = true;
            continue;
        }
        int tid = s.tweens[s.currentIndex];
        auto tw = st.tweens.find(tid);
        if (tw == st.tweens.end()) {
            s.currentIndex++;
            continue;
        }
        if (tw->second.elapsed == 0.0) resetTween(tw->second, false, true);
        updateTween(tw->second, dt);
        if (!tw->second.active) {
            s.currentIndex++;
            if (s.currentIndex < (int)s.tweens.size()) {
                int nextId = s.tweens[s.currentIndex];
                auto ntw = st.tweens.find(nextId);
                if (ntw != st.tweens.end()) resetTween(ntw->second, false, true);
            }
        }
    }
    for (auto& kv : st.parallels) {
        ParallelGroup& p = kv.second;
        if (!p.playing || p.finished) continue;
        bool allDone = true;
        for (int tid : p.tweens) {
            auto tw = st.tweens.find(tid);
            if (tw == st.tweens.end()) continue;
            if (tw->second.elapsed == 0.0) resetTween(tw->second, false, true);
            updateTween(tw->second, dt);
            if (tw->second.active) allDone = false;
        }
        if (allDone) p.finished = true;
    }
}
void cleanupFinishedTweens() {
    // no-op: handled inline during updateTweens
}

// Exposed update helpers
void updateTweensTick(double dt) {
    updateTweens(dt);
}
void cleanupTweens() {
    cleanupFinishedTweens();
}

// Public binding wrappers reuse api names above
} // namespace yuki
