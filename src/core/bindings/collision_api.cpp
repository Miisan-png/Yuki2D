#include "collision_api.hpp"
#include "state.hpp"
#include "value_utils.hpp"
#include "../renderer2d.hpp"
#include <set>

namespace yuki {
namespace {
BindingsState& st = bindingsState();
bool rectsOverlap(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh) {
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}
std::string makeAreaKey(int id, const std::string& tag) {
    return std::to_string(id) + "|" + tag;
}
bool areaOverlapsTagInternal(int id, const std::string& tag) {
    if (id < 0 || id >= (int)st.areas.size()) return false;
    const auto& A = st.areas[id];
    for (size_t i = 0; i < st.areas.size(); ++i) {
        if ((int)i == id) continue;
        if (st.areas[i].tag != tag) continue;
        const auto& B = st.areas[i];
        bool overlap = rectsOverlap(A.x, A.y, A.w, A.h, B.x, B.y, B.w, B.h);
        if (overlap) return true;
    }
    return false;
}
} // namespace

Value apiColliderCreate(const std::vector<Value>& args) {
    if (args.size() < 5) return Value::number(-1);
    Collider c;
    c.x = (float)args[0].numberVal;
    c.y = (float)args[1].numberVal;
    c.w = (float)args[2].numberVal;
    c.h = (float)args[3].numberVal;
    c.tag = args[4].toString();
    c.solid = args.size() > 5 ? valueToBool(args[5], true) : true;
    st.colliders.push_back(c);
    return Value::number((int)st.colliders.size() - 1);
}
Value apiColliderSetPos(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::nilVal();
    int id = (int)args[0].numberVal;
    if (id < 0 || id >= (int)st.colliders.size()) return Value::nilVal();
    st.colliders[id].x = (float)args[1].numberVal;
    st.colliders[id].y = (float)args[2].numberVal;
    return Value::nilVal();
}
Value apiColliderSetSize(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::nilVal();
    int id = (int)args[0].numberVal;
    if (id < 0 || id >= (int)st.colliders.size()) return Value::nilVal();
    st.colliders[id].w = (float)args[1].numberVal;
    st.colliders[id].h = (float)args[2].numberVal;
    return Value::nilVal();
}
Value apiColliderGetPos(const std::vector<Value>& args) {
    if (args.empty()) return Value::map({});
    int id = (int)args[0].numberVal;
    if (id < 0 || id >= (int)st.colliders.size()) return Value::map({});
    std::unordered_map<std::string, Value> m;
    m["x"] = Value::number(st.colliders[id].x);
    m["y"] = Value::number(st.colliders[id].y);
    return Value::map(m);
}
Value apiColliderGetSize(const std::vector<Value>& args) {
    if (args.empty()) return Value::map({});
    int id = (int)args[0].numberVal;
    if (id < 0 || id >= (int)st.colliders.size()) return Value::map({});
    std::unordered_map<std::string, Value> m;
    m["w"] = Value::number(st.colliders[id].w);
    m["h"] = Value::number(st.colliders[id].h);
    return Value::map(m);
}
Value apiColliderMove(const std::vector<Value>& args) {
    if (args.size() < 3) return Value::array({});
    int id = (int)args[0].numberVal;
    if (id < 0 || id >= (int)st.colliders.size()) return Value::array({});
    float dx = (float)args[1].numberVal;
    float dy = (float)args[2].numberVal;
    Collider& c = st.colliders[id];
    float nx = c.x + dx;
    float ny = c.y + dy;
    std::set<int> hits;
    for (size_t i = 0; i < st.colliders.size(); ++i) {
        if ((int)i == id) continue;
        const Collider& o = st.colliders[i];
        if (!rectsOverlap(nx, c.y, c.w, c.h, o.x, o.y, o.w, o.h)) continue;
        if (c.solid && o.solid) {
            if (dx > 0.0f) nx = std::min(nx, o.x - c.w);
            else if (dx < 0.0f) nx = std::max(nx, o.x + o.w);
        }
        hits.insert((int)i);
    }
    c.x = nx;
    for (size_t i = 0; i < st.colliders.size(); ++i) {
        if ((int)i == id) continue;
        const Collider& o = st.colliders[i];
        if (!rectsOverlap(c.x, ny, c.w, c.h, o.x, o.y, o.w, o.h)) continue;
        if (c.solid && o.solid) {
            if (dy > 0.0f) ny = std::min(ny, o.y - c.h);
            else if (dy < 0.0f) ny = std::max(ny, o.y + o.h);
        }
        hits.insert((int)i);
    }
    c.y = ny;

    std::vector<Value> arr;
    arr.reserve(hits.size());
    for (int hid : hits) {
        if (hid < 0 || hid >= (int)st.colliders.size()) continue;
        std::unordered_map<std::string, Value> m;
        m["id"] = Value::number(hid);
        m["tag"] = Value::string(st.colliders[hid].tag);
        arr.push_back(Value::map(m));
    }
    return Value::array(arr);
}

Value apiRectOverlaps(const std::vector<Value>& args) {
    if (args.size() < 8) return Value::boolean(false);
    float x1 = args[0].numberVal;
    float y1 = args[1].numberVal;
    float w1 = args[2].numberVal;
    float h1 = args[3].numberVal;
    float x2 = args[4].numberVal;
    float y2 = args[5].numberVal;
    float w2 = args[6].numberVal;
    float h2 = args[7].numberVal;
    bool overlap = rectsOverlap(x1, y1, w1, h1, x2, y2, w2, h2);
    return Value::boolean(overlap);
}
Value apiPointInRect(const std::vector<Value>& args) {
    if (args.size() < 6) return Value::boolean(false);
    float px = args[0].numberVal;
    float py = args[1].numberVal;
    float rx = args[2].numberVal;
    float ry = args[3].numberVal;
    float rw = args[4].numberVal;
    float rh = args[5].numberVal;
    bool inside = px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
    return Value::boolean(inside);
}

Value apiCreateAreaRect(const std::vector<Value>& args) {
    if (args.size() < 5) return Value::number(-1);
    Area a;
    a.x = args[0].numberVal;
    a.y = args[1].numberVal;
    a.w = args[2].numberVal;
    a.h = args[3].numberVal;
    a.tag = args[4].toString();
    st.areas.push_back(a);
    return Value::number((int)st.areas.size() - 1);
}
Value apiSetAreaRect(const std::vector<Value>& args) {
    if (args.size() < 5) return Value::nilVal();
    int id = (int)args[0].numberVal;
    if (id < 0 || id >= (int)st.areas.size()) return Value::nilVal();
    st.areas[id].x = args[1].numberVal;
    st.areas[id].y = args[2].numberVal;
    st.areas[id].w = args[3].numberVal;
    st.areas[id].h = args[4].numberVal;
    return Value::nilVal();
}
Value apiAreaOverlaps(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::boolean(false);
    int a = (int)args[0].numberVal;
    int b = (int)args[1].numberVal;
    if (a < 0 || b < 0 || a >= (int)st.areas.size() || b >= (int)st.areas.size()) return Value::boolean(false);
    const auto& A = st.areas[a];
    const auto& B = st.areas[b];
    bool overlap = rectsOverlap(A.x, A.y, A.w, A.h, B.x, B.y, B.w, B.h);
    return Value::boolean(overlap);
}
Value apiAreaOverlapsTag(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::boolean(false);
    int id = (int)args[0].numberVal;
    std::string tag = args[1].toString();
    return Value::boolean(areaOverlapsTagInternal(id, tag));
}
Value apiAreaEnteredTag(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::boolean(false);
    int id = (int)args[0].numberVal;
    std::string tag = args[1].toString();
    std::string key = makeAreaKey(id, tag);
    bool now = areaOverlapsTagInternal(id, tag);
    bool before = st.areaState[key];
    st.areaState[key] = now;
    return Value::boolean(now && !before);
}
Value apiAreaExitedTag(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::boolean(false);
    int id = (int)args[0].numberVal;
    std::string tag = args[1].toString();
    std::string key = makeAreaKey(id, tag);
    bool now = areaOverlapsTagInternal(id, tag);
    bool before = st.areaState[key];
    st.areaState[key] = now;
    return Value::boolean(!now && before);
}
Value apiDebugArea(const std::vector<Value>& args) {
    if (args.size() < 2 || !st.renderer) return Value::nilVal();
    int id = (int)args[0].numberVal;
    float r = args[1].numberVal;
    float g = args.size() > 2 ? args[2].numberVal : 1.0f;
    float b = args.size() > 3 ? args[3].numberVal : 0.0f;
    if (id >= 0 && id < (int)st.areas.size()) {
        const auto& A = st.areas[id];
        st.renderer->debugDrawRect(A.x, A.y, A.w, A.h, r, g, b);
    }
    return Value::nilVal();
}
} // namespace yuki
