#include "register_bindings.hpp"
#include "state.hpp"
#include "../aseprite_loader.hpp"
#include "../renderer2d.hpp"
#include "anim_api.hpp"
#include "value_utils.hpp"
#include "../log.hpp"
#include <filesystem>
#include <algorithm>

namespace yuki {
namespace {
BindingsState& st = bindingsState();

std::filesystem::path resolvePath(const std::string& rel) {
    std::filesystem::path p(rel);
    if (p.is_absolute()) return p;
    if (!st.assetBase.empty()) return std::filesystem::path(st.assetBase) / p;
    return p;
}

void buildTags(const AseData& data, BindingsState::AseAsset& asset) {
    asset.tagFrames.clear();
    asset.tagFps.clear();
    for (const auto& tag : data.tags) {
        std::vector<int> frames;
        for (int fi = tag.from; fi <= tag.to && fi < (int)data.frames.size(); ++fi) {
            frames.push_back(fi);
        }
        if (tag.direction == 1) {
            std::reverse(frames.begin(), frames.end());
        } else if (tag.direction == 2) {
            if (!frames.empty()) {
                for (int i = (int)frames.size() - 2; i >= 1; --i) {
                    frames.push_back(frames[i]);
                }
            }
        }
        if (frames.empty()) continue;
        double avgMs = 0.0;
        int count = 0;
        for (int f : frames) {
            if (f >= 0 && f < (int)data.durationsMs.size() && data.durationsMs[f] > 0) {
                avgMs += data.durationsMs[f];
                count++;
            }
        }
        double fps = 10.0;
        if (count > 0) {
            avgMs /= (double)count;
            fps = avgMs > 0.0 ? 1000.0 / avgMs : 10.0;
        }
        asset.tagFrames[tag.name] = frames;
        asset.tagFps[tag.name] = fps;
    }
}
}

Value apiAseLoad(const std::vector<Value>& args) {
    if (args.empty() || !st.renderer) return Value::number(-1);
    auto p = resolvePath(args[0].toString());
    AseData data;
    if (!loadAsepriteFile(p.string(), data)) return Value::number(-1);
    int sheetId = st.renderer->createSpriteSheetFromFrames(data.width, data.height, data.frames);
    if (sheetId < 0) return Value::number(-1);
    BindingsState::AseAsset asset;
    asset.id = st.aseCounter++;
    asset.sheetId = sheetId;
    asset.frameW = data.width;
    asset.frameH = data.height;
    asset.path = p.string();
    std::error_code ec;
    asset.lastWriteTime = std::filesystem::last_write_time(p, ec);
    buildTags(data, asset);
    st.aseAssets[asset.id] = asset;
    logInfo("ase_load: " + p.string() + " tags=" + std::to_string(asset.tagFrames.size()));
    return Value::number(asset.id);
}

Value apiAseAnim(const std::vector<Value>& args) {
    if (args.size() < 2) return Value::number(-1);
    int id = (int)args[0].numberVal;
    std::string tag = args[1].toString();
    bool loop = args.size() > 2 ? valueToBool(args[2], true) : true;
    double fpsOverride = args.size() > 3 ? args[3].numberVal : -1.0;
    auto it = st.aseAssets.find(id);
    if (it == st.aseAssets.end()) return Value::number(-1);
    const auto& asset = it->second;
    auto fit = asset.tagFrames.find(tag);
    if (fit == asset.tagFrames.end()) return Value::number(-1);
    std::vector<Value> frameVals;
    for (int f : fit->second) frameVals.push_back(Value::number(f));
    double fps = fpsOverride > 0.0 ? fpsOverride : asset.tagFps.at(tag);
    std::vector<Value> animArgs;
    animArgs.push_back(Value::number(asset.sheetId));
    animArgs.push_back(Value::array(frameVals));
    animArgs.push_back(Value::number(fps));
    animArgs.push_back(Value::boolean(loop));
    return apiAnimCreate(animArgs);
}

Value apiAseTags(const std::vector<Value>& args) {
    if (args.empty()) return Value::array({});
    int id = (int)args[0].numberVal;
    auto it = st.aseAssets.find(id);
    if (it == st.aseAssets.end()) return Value::array({});
    std::vector<Value> names;
    for (const auto& kv : it->second.tagFrames) {
        names.push_back(Value::string(kv.first));
    }
    return Value::array(names);
}

void registerAseBuiltins(std::unordered_map<std::string, NativeFn>& builtins) {
    builtins["ase_load"] = apiAseLoad;
    builtins["ase_anim"] = apiAseAnim;
    builtins["ase_tags"] = apiAseTags;
}
} // namespace yuki
