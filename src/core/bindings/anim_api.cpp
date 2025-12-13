#include "anim_api.hpp"
#include "state.hpp"
#include "value_utils.hpp"
#include "../renderer2d.hpp"

namespace yuki {
namespace {
BindingsState& st = bindingsState();

Animation* getAnimation(int id) {
    auto it = st.animations.find(id);
    if (it == st.animations.end()) return nullptr;
    return &it->second;
}
} // namespace

Value apiAnimCreate(const std::vector<Value>& args) {
    if (args.size() < 4) return Value::number(-1);
    int sheetId = (int)args[0].numberVal;
    std::vector<int> frames;
    if (args[1].isArray() && args[1].arrayPtr) {
        const auto& arr = *args[1].arrayPtr;
        for (const auto& item : arr) {
            if (item.isNumber()) frames.push_back((int)item.numberVal);
        }
    }
    double fps = args[2].numberVal;
    bool loop = valueToBool(args[3]);

    Animation anim;
    anim.id = st.animationCounter++;
    anim.sheetId = sheetId;
    anim.frames = frames;
    anim.fps = fps;
    anim.loop = loop;
    st.animations[anim.id] = anim;
    return Value::number(anim.id);
}
Value apiAnimPlay(const std::vector<Value>& args) {
    if (args.size() < 1) return Value::nilVal();
    int id = (int)args[0].numberVal;
    bool reset = args.size() > 1 ? valueToBool(args[1], true) : true;
    auto* anim = getAnimation(id);
    if (!anim) return Value::nilVal();
    if (reset) {
        anim->currentIndex = 0;
        anim->accumulator = 0.0;
    } else if (!anim->frames.empty() && anim->currentIndex >= (int)anim->frames.size()) {
        anim->currentIndex %= (int)anim->frames.size();
    }
    anim->playing = true;
    return Value::nilVal();
}
Value apiAnimStop(const std::vector<Value>& args) {
    if (args.empty()) return Value::nilVal();
    auto* anim = getAnimation((int)args[0].numberVal);
    if (anim) anim->playing = false;
    return Value::nilVal();
}
Value apiAnimReset(const std::vector<Value>& args) {
    if (args.empty()) return Value::nilVal();
    auto* anim = getAnimation((int)args[0].numberVal);
    if (anim) {
        anim->currentIndex = 0;
        anim->accumulator = 0.0;
    }
    return Value::nilVal();
}
Value apiAnimSetPosition(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::nilVal();
    auto* anim = getAnimation((int)args[0].numberVal);
    if (anim) {
        anim->transform.x = args[1].numberVal;
        anim->transform.y = args[2].numberVal;
    }
    return Value::nilVal();
}
Value apiAnimSetOrigin(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::nilVal();
    auto* anim = getAnimation((int)args[0].numberVal);
    if (anim) {
        anim->transform.originX = args[1].numberVal;
        anim->transform.originY = args[2].numberVal;
    }
    return Value::nilVal();
}
Value apiAnimSetScale(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::nilVal();
    auto* anim = getAnimation((int)args[0].numberVal);
    if (anim) {
        anim->transform.scaleX = args[1].numberVal;
        anim->transform.scaleY = args[2].numberVal;
    }
    return Value::nilVal();
}
Value apiAnimSetRotation(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::nilVal();
    auto* anim = getAnimation((int)args[0].numberVal);
    if (anim) anim->transform.rotationDeg = args[1].numberVal;
    return Value::nilVal();
}
Value apiAnimSetFlip(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::nilVal();
    auto* anim = getAnimation((int)args[0].numberVal);
    if (anim) {
        bool fx = valueToBool(args[1]);
        bool fy = args.size() > 2 ? valueToBool(args[2]) : false;
        anim->transform.flipX = fx;
        anim->transform.flipY = fy;
    }
    return Value::nilVal();
}
Value apiAnimSetAlpha(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::nilVal();
    auto* anim = getAnimation((int)args[0].numberVal);
    if (anim) anim->alpha = args[1].numberVal;
    return Value::nilVal();
}
Value apiAnimDraw(const std::vector<Value>& args) {
    if (args.size() < 1 || !st.renderer) return Value::nilVal();
    auto* anim = getAnimation((int)args[0].numberVal);
    if (!anim || anim->sheetId < 0) return Value::nilVal();
    if (anim->frames.empty()) return Value::nilVal();
    int frame = anim->frames[anim->currentIndex % (int)anim->frames.size()];
    st.renderer->drawSpriteFrame(anim->sheetId, frame, anim->transform.x, anim->transform.y, anim->transform.rotationDeg, anim->transform.scaleX, anim->transform.scaleY, anim->transform.flipX, anim->transform.flipY, anim->transform.originX, anim->transform.originY, anim->alpha);
    return Value::nilVal();
}

Value apiAnimGetPosition(const std::vector<Value>& args) {
    if (args.empty()) return Value::map({});
    auto* anim = getAnimation((int)args[0].numberVal);
    if (!anim) return Value::map({});
    std::unordered_map<std::string, Value> m;
    m["x"] = Value::number(anim->transform.x);
    m["y"] = Value::number(anim->transform.y);
    return Value::map(m);
}

Value apiAnimGetScale(const std::vector<Value>& args) {
    if (args.empty()) return Value::map({});
    auto* anim = getAnimation((int)args[0].numberVal);
    if (!anim) return Value::map({});
    std::unordered_map<std::string, Value> m;
    m["x"] = Value::number(anim->transform.scaleX);
    m["y"] = Value::number(anim->transform.scaleY);
    return Value::map(m);
}

Value apiAnimGetRotation(const std::vector<Value>& args) {
    if (args.empty()) return Value::number(0);
    auto* anim = getAnimation((int)args[0].numberVal);
    if (!anim) return Value::number(0);
    return Value::number(anim->transform.rotationDeg);
}

Value apiAnimGetAlpha(const std::vector<Value>& args) {
    if (args.empty()) return Value::number(1);
    auto* anim = getAnimation((int)args[0].numberVal);
    if (!anim) return Value::number(1);
    return Value::number(anim->alpha);
}

void updateAnimationsTick(double dt) {
    for (auto& kv : st.animations) {
        Animation& a = kv.second;
        if (!a.playing || a.frames.empty() || a.fps <= 0.0) continue;
        if (a.currentIndex >= (int)a.frames.size()) a.currentIndex %= (int)a.frames.size();
        a.accumulator += dt;
        double frameTime = 1.0 / a.fps;
        while (a.accumulator >= frameTime) {
            a.accumulator -= frameTime;
            a.currentIndex++;
            if (a.currentIndex >= (int)a.frames.size()) {
                if (a.loop) a.currentIndex = 0;
                else { a.currentIndex = (int)a.frames.size() - 1; a.playing = false; break; }
            }
        }
    }
}
} // namespace yuki
