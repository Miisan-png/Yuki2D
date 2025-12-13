#include "ui_api.hpp"
#include "value_utils.hpp"
#include "../renderer2d.hpp"
#include "../engine_bindings.hpp"
#include "state.hpp"
#include <imgui.h>
#include <imgui_impl_opengl2.h>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>

namespace yuki {
namespace {
BindingsState& st = bindingsState();

bool uiEnabled() {
    return ImGui::GetCurrentContext() != nullptr;
}

std::vector<ImFont*> gFonts;
bool gHasBaseStyle = false;
ImGuiStyle gBaseStyle;
float gUiScale = 1.25f;

void applyThemeYuki() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 10.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 6.0f;

    ImVec4 bg0 = ImVec4(0.08f, 0.09f, 0.11f, 1.00f);
    ImVec4 bg1 = ImVec4(0.11f, 0.12f, 0.15f, 1.00f);
    ImVec4 bg2 = ImVec4(0.14f, 0.15f, 0.18f, 1.00f);
    ImVec4 text = ImVec4(0.92f, 0.94f, 0.98f, 1.00f);
    ImVec4 mutetext = ImVec4(0.60f, 0.65f, 0.72f, 1.00f);
    ImVec4 accent = ImVec4(0.55f, 0.78f, 0.95f, 1.00f);
    ImVec4 accent2 = ImVec4(0.38f, 0.62f, 0.82f, 1.00f);

    ImVec4* c = style.Colors;
    c[ImGuiCol_Text] = text;
    c[ImGuiCol_TextDisabled] = mutetext;
    c[ImGuiCol_WindowBg] = bg0;
    c[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_PopupBg] = bg1;
    c[ImGuiCol_Border] = ImVec4(0.20f, 0.22f, 0.26f, 1.00f);
    c[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_FrameBg] = bg1;
    c[ImGuiCol_FrameBgHovered] = bg2;
    c[ImGuiCol_FrameBgActive] = ImVec4(bg2.x + 0.03f, bg2.y + 0.03f, bg2.z + 0.03f, 1.0f);
    c[ImGuiCol_TitleBg] = bg0;
    c[ImGuiCol_TitleBgActive] = bg1;
    c[ImGuiCol_TitleBgCollapsed] = bg0;
    c[ImGuiCol_MenuBarBg] = bg1;
    c[ImGuiCol_ScrollbarBg] = bg0;
    c[ImGuiCol_ScrollbarGrab] = bg2;
    c[ImGuiCol_ScrollbarGrabHovered] = accent2;
    c[ImGuiCol_ScrollbarGrabActive] = accent;
    c[ImGuiCol_CheckMark] = accent;
    c[ImGuiCol_SliderGrab] = accent2;
    c[ImGuiCol_SliderGrabActive] = accent;
    c[ImGuiCol_Button] = bg1;
    c[ImGuiCol_ButtonHovered] = ImVec4(bg1.x + 0.06f, bg1.y + 0.07f, bg1.z + 0.09f, 1.0f);
    c[ImGuiCol_ButtonActive] = accent2;
    c[ImGuiCol_Header] = bg1;
    c[ImGuiCol_HeaderHovered] = ImVec4(bg1.x + 0.05f, bg1.y + 0.06f, bg1.z + 0.08f, 1.0f);
    c[ImGuiCol_HeaderActive] = accent2;
    c[ImGuiCol_Separator] = ImVec4(0.22f, 0.24f, 0.28f, 1.00f);
    c[ImGuiCol_SeparatorHovered] = accent2;
    c[ImGuiCol_SeparatorActive] = accent;
    c[ImGuiCol_ResizeGrip] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_ResizeGripHovered] = accent2;
    c[ImGuiCol_ResizeGripActive] = accent;
    c[ImGuiCol_Tab] = bg1;
    c[ImGuiCol_TabHovered] = accent2;
    c[ImGuiCol_TabActive] = bg2;
    c[ImGuiCol_TabUnfocused] = bg1;
    c[ImGuiCol_TabUnfocusedActive] = bg2;
    c[ImGuiCol_PlotLines] = accent2;
    c[ImGuiCol_PlotHistogram] = accent2;
    c[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.35f);
}

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    return s;
}

void captureBaseStyle() {
    gBaseStyle = ImGui::GetStyle();
    gHasBaseStyle = true;
}

void applyScale(float scale) {
    if (!gHasBaseStyle) captureBaseStyle();
    ImGuiStyle style = gBaseStyle;
    style.ScaleAllSizes(scale);
    ImGui::GetStyle() = style;
    ImGui::GetIO().FontGlobalScale = scale;
    gUiScale = scale;
}
}

Value apiUiEnabled(const std::vector<Value>&) {
    return Value::boolean(uiEnabled());
}

Value apiUiConstants(const std::vector<Value>&) {
    std::unordered_map<std::string, Value> out;
    std::unordered_map<std::string, Value> cond;
    cond["Always"] = Value::number((double)ImGuiCond_Always);
    cond["Once"] = Value::number((double)ImGuiCond_Once);
    cond["FirstUseEver"] = Value::number((double)ImGuiCond_FirstUseEver);
    cond["Appearing"] = Value::number((double)ImGuiCond_Appearing);
    out["Cond"] = Value::map(cond);

    std::unordered_map<std::string, Value> wf;
    wf["None"] = Value::number((double)ImGuiWindowFlags_None);
    wf["NoTitleBar"] = Value::number((double)ImGuiWindowFlags_NoTitleBar);
    wf["NoResize"] = Value::number((double)ImGuiWindowFlags_NoResize);
    wf["NoMove"] = Value::number((double)ImGuiWindowFlags_NoMove);
    wf["NoScrollbar"] = Value::number((double)ImGuiWindowFlags_NoScrollbar);
    wf["NoScrollWithMouse"] = Value::number((double)ImGuiWindowFlags_NoScrollWithMouse);
    wf["NoCollapse"] = Value::number((double)ImGuiWindowFlags_NoCollapse);
    wf["AlwaysAutoResize"] = Value::number((double)ImGuiWindowFlags_AlwaysAutoResize);
    wf["NoBackground"] = Value::number((double)ImGuiWindowFlags_NoBackground);
    wf["NoSavedSettings"] = Value::number((double)ImGuiWindowFlags_NoSavedSettings);
    wf["NoMouseInputs"] = Value::number((double)ImGuiWindowFlags_NoMouseInputs);
    wf["MenuBar"] = Value::number((double)ImGuiWindowFlags_MenuBar);
    wf["HorizontalScrollbar"] = Value::number((double)ImGuiWindowFlags_HorizontalScrollbar);
    wf["NoFocusOnAppearing"] = Value::number((double)ImGuiWindowFlags_NoFocusOnAppearing);
    wf["NoBringToFrontOnFocus"] = Value::number((double)ImGuiWindowFlags_NoBringToFrontOnFocus);
    wf["AlwaysVerticalScrollbar"] = Value::number((double)ImGuiWindowFlags_AlwaysVerticalScrollbar);
    wf["AlwaysHorizontalScrollbar"] = Value::number((double)ImGuiWindowFlags_AlwaysHorizontalScrollbar);
    wf["NoNavInputs"] = Value::number((double)ImGuiWindowFlags_NoNavInputs);
    wf["NoNavFocus"] = Value::number((double)ImGuiWindowFlags_NoNavFocus);
    wf["UnsavedDocument"] = Value::number((double)ImGuiWindowFlags_UnsavedDocument);
    out["WindowFlags"] = Value::map(wf);

    std::unordered_map<std::string, Value> tf;
    tf["None"] = Value::number((double)ImGuiTableFlags_None);
    tf["Resizable"] = Value::number((double)ImGuiTableFlags_Resizable);
    tf["Reorderable"] = Value::number((double)ImGuiTableFlags_Reorderable);
    tf["Hideable"] = Value::number((double)ImGuiTableFlags_Hideable);
    tf["Sortable"] = Value::number((double)ImGuiTableFlags_Sortable);
    tf["RowBg"] = Value::number((double)ImGuiTableFlags_RowBg);
    tf["Borders"] = Value::number((double)ImGuiTableFlags_Borders);
    tf["BordersInnerV"] = Value::number((double)ImGuiTableFlags_BordersInnerV);
    tf["BordersInnerH"] = Value::number((double)ImGuiTableFlags_BordersInnerH);
    tf["BordersOuterV"] = Value::number((double)ImGuiTableFlags_BordersOuterV);
    tf["BordersOuterH"] = Value::number((double)ImGuiTableFlags_BordersOuterH);
    tf["NoBordersInBody"] = Value::number((double)ImGuiTableFlags_NoBordersInBody);
    tf["ScrollY"] = Value::number((double)ImGuiTableFlags_ScrollY);
    tf["ScrollX"] = Value::number((double)ImGuiTableFlags_ScrollX);
    tf["SizingFixedFit"] = Value::number((double)ImGuiTableFlags_SizingFixedFit);
    tf["SizingStretchSame"] = Value::number((double)ImGuiTableFlags_SizingStretchSame);
    out["TableFlags"] = Value::map(tf);

    std::unordered_map<std::string, Value> tbf;
    tbf["None"] = Value::number((double)ImGuiTabBarFlags_None);
    tbf["Reorderable"] = Value::number((double)ImGuiTabBarFlags_Reorderable);
    tbf["AutoSelectNewTabs"] = Value::number((double)ImGuiTabBarFlags_AutoSelectNewTabs);
    tbf["TabListPopupButton"] = Value::number((double)ImGuiTabBarFlags_TabListPopupButton);
    tbf["FittingPolicyResizeDown"] = Value::number((double)ImGuiTabBarFlags_FittingPolicyResizeDown);
    tbf["FittingPolicyScroll"] = Value::number((double)ImGuiTabBarFlags_FittingPolicyScroll);
    out["TabBarFlags"] = Value::map(tbf);

    std::unordered_map<std::string, Value> tif;
    tif["None"] = Value::number((double)ImGuiTabItemFlags_None);
    tif["UnsavedDocument"] = Value::number((double)ImGuiTabItemFlags_UnsavedDocument);
    tif["SetSelected"] = Value::number((double)ImGuiTabItemFlags_SetSelected);
    tif["NoCloseWithMiddleMouseButton"] = Value::number((double)ImGuiTabItemFlags_NoCloseWithMiddleMouseButton);
    out["TabItemFlags"] = Value::map(tif);

    return Value::map(out);
}

Value apiUiBegin(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::boolean(false);
    if (args.empty()) return Value::boolean(false);
    std::string name = args[0].toString();
    bool open = ImGui::Begin(name.c_str());
    return Value::boolean(open);
}

Value apiUiBeginEx(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::map({});
    if (args.empty()) return Value::map({});
    std::string name = args[0].toString();
    bool open = true;
    int flags = 0;
    if (args.size() > 1 && args[1].isBool()) open = args[1].boolVal;
    if (args.size() > 2 && args[2].isNumber()) flags = (int)args[2].numberVal;
    bool visible = ImGui::Begin(name.c_str(), &open, flags);
    std::unordered_map<std::string, Value> out;
    out["open"] = Value::boolean(open);
    out["visible"] = Value::boolean(visible);
    return Value::map(out);
}

Value apiUiEnd(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::nilVal();
    ImGui::End();
    return Value::nilVal();
}

Value apiUiText(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::nilVal();
    if (args.empty()) return Value::nilVal();
    std::string text = args[0].toString();
    ImGui::TextUnformatted(text.c_str());
    return Value::nilVal();
}

Value apiUiSeparator(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::nilVal();
    ImGui::Separator();
    return Value::nilVal();
}

Value apiUiSameLine(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::nilVal();
    ImGui::SameLine();
    return Value::nilVal();
}

Value apiUiSpacing(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::nilVal();
    ImGui::Spacing();
    return Value::nilVal();
}

Value apiUiNewLine(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::nilVal();
    ImGui::NewLine();
    return Value::nilVal();
}

Value apiUiBeginChild(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::boolean(false);
    if (args.empty()) return Value::boolean(false);
    std::string name = args[0].toString();
    float w = args.size() > 1 && args[1].isNumber() ? (float)args[1].numberVal : 0.0f;
    float h = args.size() > 2 && args[2].isNumber() ? (float)args[2].numberVal : 0.0f;
    bool border = args.size() > 3 ? valueToBool(args[3]) : false;
    int flags = args.size() > 4 && args[4].isNumber() ? (int)args[4].numberVal : 0;
    bool ok = ImGui::BeginChild(name.c_str(), ImVec2(w, h), border, flags);
    return Value::boolean(ok);
}

Value apiUiEndChild(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::nilVal();
    ImGui::EndChild();
    return Value::nilVal();
}

Value apiUiCollapsingHeader(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::boolean(false);
    if (args.empty()) return Value::boolean(false);
    std::string label = args[0].toString();
    bool defaultOpen = args.size() > 1 ? valueToBool(args[1]) : false;
    ImGuiTreeNodeFlags flags = defaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0;
    return Value::boolean(ImGui::CollapsingHeader(label.c_str(), flags));
}

Value apiUiTreeNode(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::boolean(false);
    if (args.empty()) return Value::boolean(false);
    std::string label = args[0].toString();
    return Value::boolean(ImGui::TreeNode(label.c_str()));
}

Value apiUiTreePop(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::nilVal();
    ImGui::TreePop();
    return Value::nilVal();
}

Value apiUiSelectable(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::boolean(false);
    if (args.empty()) return Value::boolean(false);
    std::string label = args[0].toString();
    bool selected = args.size() > 1 ? valueToBool(args[1]) : false;
    bool clicked = ImGui::Selectable(label.c_str(), selected);
    return Value::boolean(clicked);
}

Value apiUiButton(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::boolean(false);
    if (args.empty()) return Value::boolean(false);
    std::string label = args[0].toString();
    return Value::boolean(ImGui::Button(label.c_str()));
}

Value apiUiCheckbox(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::boolean(false);
    if (args.size() < 2) return Value::boolean(false);
    std::string label = args[0].toString();
    bool v = valueToBool(args[1]);
    ImGui::Checkbox(label.c_str(), &v);
    return Value::boolean(v);
}

Value apiUiSliderFloat(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::number(0);
    if (args.size() < 4) return Value::number(0);
    std::string label = args[0].toString();
    float v = (float)args[1].numberVal;
    float vmin = (float)args[2].numberVal;
    float vmax = (float)args[3].numberVal;
    ImGui::SliderFloat(label.c_str(), &v, vmin, vmax);
    return Value::number((double)v);
}

Value apiUiSliderInt(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::number(0);
    if (args.size() < 4) return Value::number(0);
    std::string label = args[0].toString();
    int v = (int)args[1].numberVal;
    int vmin = (int)args[2].numberVal;
    int vmax = (int)args[3].numberVal;
    ImGui::SliderInt(label.c_str(), &v, vmin, vmax);
    return Value::number((double)v);
}

Value apiUiDragFloat(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::number(0);
    if (args.size() < 2) return Value::number(0);
    std::string label = args[0].toString();
    float v = (float)args[1].numberVal;
    float speed = args.size() > 2 && args[2].isNumber() ? (float)args[2].numberVal : 1.0f;
    float vmin = args.size() > 3 && args[3].isNumber() ? (float)args[3].numberVal : 0.0f;
    float vmax = args.size() > 4 && args[4].isNumber() ? (float)args[4].numberVal : 0.0f;
    ImGui::DragFloat(label.c_str(), &v, speed, vmin, vmax);
    return Value::number((double)v);
}

Value apiUiDragInt(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::number(0);
    if (args.size() < 2) return Value::number(0);
    std::string label = args[0].toString();
    int v = (int)args[1].numberVal;
    float speed = args.size() > 2 && args[2].isNumber() ? (float)args[2].numberVal : 1.0f;
    int vmin = args.size() > 3 && args[3].isNumber() ? (int)args[3].numberVal : 0;
    int vmax = args.size() > 4 && args[4].isNumber() ? (int)args[4].numberVal : 0;
    ImGui::DragInt(label.c_str(), &v, speed, vmin, vmax);
    return Value::number((double)v);
}

Value apiUiCombo(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::number(0);
    if (args.size() < 3) return Value::number(0);
    std::string label = args[0].toString();
    int current = (int)args[1].numberVal;
    if (!args[2].isArray() || !args[2].arrayPtr) return Value::number((double)current);
    if (args[2].arrayPtr->empty()) return Value::number((double)current);
    std::vector<std::string> items;
    items.reserve(args[2].arrayPtr->size());
    for (const auto& v : *args[2].arrayPtr) items.push_back(v.toString());
    std::vector<const char*> cstrs;
    cstrs.reserve(items.size());
    for (const auto& s : items) cstrs.push_back(s.c_str());
    if (current < 0) current = 0;
    if (current >= (int)cstrs.size()) current = (int)cstrs.size() - 1;
    ImGui::Combo(label.c_str(), &current, cstrs.data(), (int)cstrs.size());
    return Value::number((double)current);
}

Value apiUiInputText(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::string("");
    if (args.size() < 2) return Value::string("");
    std::string label = args[0].toString();
    std::string value = args[1].toString();
    int cap = 256;
    if (args.size() > 2 && args[2].isNumber()) cap = std::max(16, (int)args[2].numberVal);
    std::vector<char> buf((size_t)cap);
    std::fill(buf.begin(), buf.end(), 0);
    size_t n = std::min(value.size(), buf.size() - 1);
    std::copy(value.begin(), value.begin() + (long)n, buf.begin());
    ImGui::InputText(label.c_str(), buf.data(), buf.size());
    return Value::string(std::string(buf.data()));
}

Value apiUiInputTextEx(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::map({});
    if (args.size() < 2) return Value::map({});
    std::string label = args[0].toString();
    std::string value = args[1].toString();
    int cap = 256;
    if (args.size() > 2 && args[2].isNumber()) cap = std::max(16, (int)args[2].numberVal);
    std::vector<char> buf((size_t)cap);
    std::fill(buf.begin(), buf.end(), 0);
    size_t n = std::min(value.size(), buf.size() - 1);
    std::copy(value.begin(), value.begin() + (long)n, buf.begin());
    bool changed = ImGui::InputText(label.c_str(), buf.data(), buf.size());
    std::unordered_map<std::string, Value> out;
    out["changed"] = Value::boolean(changed);
    out["value"] = Value::string(std::string(buf.data()));
    return Value::map(out);
}

Value apiUiTextColored(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::nilVal();
    if (args.size() < 5) return Value::nilVal();
    float r = (float)args[0].numberVal;
    float g = (float)args[1].numberVal;
    float b = (float)args[2].numberVal;
    float a = (float)args[3].numberVal;
    std::string text = args[4].toString();
    ImGui::TextColored(ImVec4(r, g, b, a), "%s", text.c_str());
    return Value::nilVal();
}

Value apiUiColorEdit4(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::array({});
    if (args.size() < 5) return Value::array({});
    std::string label = args[0].toString();
    float col[4] = {(float)args[1].numberVal, (float)args[2].numberVal, (float)args[3].numberVal, (float)args[4].numberVal};
    ImGui::ColorEdit4(label.c_str(), col);
    std::vector<Value> out;
    out.push_back(Value::number(col[0]));
    out.push_back(Value::number(col[1]));
    out.push_back(Value::number(col[2]));
    out.push_back(Value::number(col[3]));
    return Value::array(out);
}

Value apiUiProgressBar(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::nilVal();
    if (args.empty()) return Value::nilVal();
    float frac = (float)args[0].numberVal;
    std::string overlay = args.size() > 1 ? args[1].toString() : "";
    ImGui::ProgressBar(frac, ImVec2(0.0f, 0.0f), overlay.empty() ? nullptr : overlay.c_str());
    return Value::nilVal();
}

Value apiUiBeginMainMenuBar(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::boolean(false);
    return Value::boolean(ImGui::BeginMainMenuBar());
}

Value apiUiEndMainMenuBar(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::nilVal();
    ImGui::EndMainMenuBar();
    return Value::nilVal();
}

Value apiUiBeginMenuBar(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::boolean(false);
    return Value::boolean(ImGui::BeginMenuBar());
}

Value apiUiEndMenuBar(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::nilVal();
    ImGui::EndMenuBar();
    return Value::nilVal();
}

Value apiUiBeginMenu(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::boolean(false);
    if (args.empty()) return Value::boolean(false);
    std::string label = args[0].toString();
    bool enabled = args.size() > 1 ? valueToBool(args[1]) : true;
    return Value::boolean(ImGui::BeginMenu(label.c_str(), enabled));
}

Value apiUiEndMenu(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::nilVal();
    ImGui::EndMenu();
    return Value::nilVal();
}

Value apiUiMenuItem(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::boolean(false);
    if (args.empty()) return Value::boolean(false);
    std::string label = args[0].toString();
    std::string shortcut = args.size() > 1 ? args[1].toString() : "";
    bool selected = args.size() > 2 ? valueToBool(args[2]) : false;
    bool enabled = args.size() > 3 ? valueToBool(args[3]) : true;
    bool clicked = ImGui::MenuItem(label.c_str(), shortcut.empty() ? nullptr : shortcut.c_str(), selected, enabled);
    return Value::boolean(clicked);
}

Value apiUiSetNextWindowPos(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::nilVal();
    if (args.size() < 2) return Value::nilVal();
    float x = (float)args[0].numberVal;
    float y = (float)args[1].numberVal;
    int cond = args.size() > 2 && args[2].isNumber() ? (int)args[2].numberVal : (int)ImGuiCond_Once;
    ImGui::SetNextWindowPos(ImVec2(x, y), cond);
    return Value::nilVal();
}

Value apiUiSetNextWindowSize(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::nilVal();
    if (args.size() < 2) return Value::nilVal();
    float w = (float)args[0].numberVal;
    float h = (float)args[1].numberVal;
    int cond = args.size() > 2 && args[2].isNumber() ? (int)args[2].numberVal : (int)ImGuiCond_Once;
    ImGui::SetNextWindowSize(ImVec2(w, h), cond);
    return Value::nilVal();
}

Value apiUiSetNextItemWidth(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::nilVal();
    if (args.empty() || !args[0].isNumber()) return Value::nilVal();
    ImGui::SetNextItemWidth((float)args[0].numberVal);
    return Value::nilVal();
}

Value apiUiBeginTabBar(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::boolean(false);
    if (args.empty()) return Value::boolean(false);
    std::string id = args[0].toString();
    int flags = args.size() > 1 && args[1].isNumber() ? (int)args[1].numberVal : 0;
    return Value::boolean(ImGui::BeginTabBar(id.c_str(), flags));
}

Value apiUiEndTabBar(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::nilVal();
    ImGui::EndTabBar();
    return Value::nilVal();
}

Value apiUiBeginTabItemEx(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::map({});
    if (args.empty()) return Value::map({});
    std::string label = args[0].toString();
    bool open = true;
    int flags = 0;
    if (args.size() > 1 && args[1].isBool()) open = args[1].boolVal;
    if (args.size() > 2 && args[2].isNumber()) flags = (int)args[2].numberVal;
    bool visible = ImGui::BeginTabItem(label.c_str(), &open, flags);
    std::unordered_map<std::string, Value> out;
    out["open"] = Value::boolean(open);
    out["visible"] = Value::boolean(visible);
    return Value::map(out);
}

Value apiUiEndTabItem(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::nilVal();
    ImGui::EndTabItem();
    return Value::nilVal();
}

Value apiUiWindow(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::map({});
    if (args.empty()) return Value::map({});
    std::string name = args[0].toString();
    bool open = true;
    int flags = 0;
    Value cb = Value::nilVal();
    if (args.size() > 1 && args[1].isBool()) open = args[1].boolVal;
    if (args.size() > 2 && args[2].isNumber()) flags = (int)args[2].numberVal;
    if (args.size() > 3) cb = args[3];
    bool visible = ImGui::Begin(name.c_str(), &open, flags);
    if (visible && cb.isFunction() && st.interpreter) {
        std::vector<Value> noArgs;
        st.interpreter->callFunction(cb, noArgs);
    }
    ImGui::End();
    std::unordered_map<std::string, Value> out;
    out["open"] = Value::boolean(open);
    out["visible"] = Value::boolean(visible);
    return Value::map(out);
}

Value apiUiDemoWindow(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::boolean(false);
    bool open = true;
    if (!args.empty()) open = valueToBool(args[0]);
    ImGui::ShowDemoWindow(&open);
    return Value::boolean(open);
}

Value apiUiImageSprite(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::nilVal();
    BindingsState& st = bindingsState();
    if (!st.renderer) return Value::nilVal();
    if (args.size() < 3) return Value::nilVal();
    int spriteId = (int)args[0].numberVal;
    float w = (float)args[1].numberVal;
    float h = (float)args[2].numberVal;
    unsigned int tex = st.renderer->getSpriteGlHandle(spriteId);
    if (tex == 0) return Value::nilVal();
    ImTextureID tid = (ImTextureID)(intptr_t)tex;
    ImGui::Image(tid, ImVec2(w, h));
    return Value::nilVal();
}

Value apiUiImageSpriteSheet(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::nilVal();
    BindingsState& st = bindingsState();
    if (!st.renderer) return Value::nilVal();
    if (args.size() < 3) return Value::nilVal();
    int sheetId = (int)args[0].numberVal;
    float w = (float)args[1].numberVal;
    float h = (float)args[2].numberVal;
    unsigned int tex = st.renderer->getSpriteSheetGlHandle(sheetId);
    if (tex == 0) return Value::nilVal();
    ImTextureID tid = (ImTextureID)(intptr_t)tex;
    ImGui::Image(tid, ImVec2(w, h));
    return Value::nilVal();
}

Value apiUiImageFont(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::nilVal();
    BindingsState& st = bindingsState();
    if (!st.renderer) return Value::nilVal();
    if (args.size() < 3) return Value::nilVal();
    int fontId = (int)args[0].numberVal;
    float w = (float)args[1].numberVal;
    float h = (float)args[2].numberVal;
    unsigned int tex = st.renderer->getFontGlHandle(fontId);
    if (tex == 0) return Value::nilVal();
    ImTextureID tid = (ImTextureID)(intptr_t)tex;
    ImGui::Image(tid, ImVec2(w, h));
    return Value::nilVal();
}

Value apiUiBeginTable(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::boolean(false);
    if (args.size() < 2) return Value::boolean(false);
    std::string id = args[0].toString();
    int cols = (int)args[1].numberVal;
    int flags = args.size() > 2 && args[2].isNumber() ? (int)args[2].numberVal : 0;
    return Value::boolean(ImGui::BeginTable(id.c_str(), cols, flags));
}

Value apiUiEndTable(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::nilVal();
    ImGui::EndTable();
    return Value::nilVal();
}

Value apiUiTableSetupColumn(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::nilVal();
    if (args.empty()) return Value::nilVal();
    std::string label = args[0].toString();
    int flags = args.size() > 1 && args[1].isNumber() ? (int)args[1].numberVal : 0;
    float initWidthOrWeight = args.size() > 2 && args[2].isNumber() ? (float)args[2].numberVal : 0.0f;
    ImGui::TableSetupColumn(label.c_str(), flags, initWidthOrWeight);
    return Value::nilVal();
}

Value apiUiTableHeadersRow(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::nilVal();
    ImGui::TableHeadersRow();
    return Value::nilVal();
}

Value apiUiTableNextRow(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::nilVal();
    ImGui::TableNextRow();
    return Value::nilVal();
}

Value apiUiTableSetColumnIndex(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::boolean(false);
    if (args.empty()) return Value::boolean(false);
    int idx = (int)args[0].numberVal;
    return Value::boolean(ImGui::TableSetColumnIndex(idx));
}

Value apiUiTableNextColumn(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::boolean(false);
    return Value::boolean(ImGui::TableNextColumn());
}

Value apiUiOpenPopup(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::nilVal();
    if (args.empty()) return Value::nilVal();
    std::string id = args[0].toString();
    ImGui::OpenPopup(id.c_str());
    return Value::nilVal();
}

Value apiUiBeginPopupModal(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::boolean(false);
    if (args.empty()) return Value::boolean(false);
    std::string name = args[0].toString();
    bool open = true;
    if (args.size() > 1 && args[1].isBool()) open = args[1].boolVal;
    return Value::boolean(ImGui::BeginPopupModal(name.c_str(), &open));
}

Value apiUiBeginPopupModalEx(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::map({});
    if (args.empty()) return Value::map({});
    std::string name = args[0].toString();
    bool open = true;
    int flags = 0;
    if (args.size() > 1 && args[1].isBool()) open = args[1].boolVal;
    if (args.size() > 2 && args[2].isNumber()) flags = (int)args[2].numberVal;
    bool visible = ImGui::BeginPopupModal(name.c_str(), &open, flags);
    std::unordered_map<std::string, Value> out;
    out["open"] = Value::boolean(open);
    out["visible"] = Value::boolean(visible);
    return Value::map(out);
}

Value apiUiEndPopup(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::nilVal();
    ImGui::EndPopup();
    return Value::nilVal();
}

Value apiUiCloseCurrentPopup(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::nilVal();
    ImGui::CloseCurrentPopup();
    return Value::nilVal();
}

Value apiUiSetTooltip(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::nilVal();
    if (args.empty()) return Value::nilVal();
    std::string t = args[0].toString();
    ImGui::SetTooltip("%s", t.c_str());
    return Value::nilVal();
}

Value apiUiModal(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::map({});
    if (args.empty()) return Value::map({});
    std::string name = args[0].toString();
    bool open = true;
    int flags = 0;
    Value cb = Value::nilVal();
    if (args.size() > 1 && args[1].isBool()) open = args[1].boolVal;
    if (args.size() > 2 && args[2].isNumber()) flags = (int)args[2].numberVal;
    if (args.size() > 3) cb = args[3];
    bool visible = ImGui::BeginPopupModal(name.c_str(), &open, flags);
    if (visible && cb.isFunction() && st.interpreter) {
        std::vector<Value> noArgs;
        st.interpreter->callFunction(cb, noArgs);
    }
    if (visible) ImGui::EndPopup();
    std::unordered_map<std::string, Value> out;
    out["open"] = Value::boolean(open);
    out["visible"] = Value::boolean(visible);
    return Value::map(out);
}

Value apiUiWantCaptureKeyboard(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::boolean(false);
    return Value::boolean(ImGui::GetIO().WantCaptureKeyboard);
}

Value apiUiWantCaptureMouse(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::boolean(false);
    return Value::boolean(ImGui::GetIO().WantCaptureMouse);
}

Value apiUiSetTheme(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::nilVal();
    std::string name = args.empty() ? "yuki" : toLower(args[0].toString());
    if (name == "dark") ImGui::StyleColorsDark();
    else if (name == "light") ImGui::StyleColorsLight();
    else if (name == "classic") ImGui::StyleColorsClassic();
    else {
        ImGui::StyleColorsDark();
        applyThemeYuki();
    }
    captureBaseStyle();
    applyScale(gUiScale);
    return Value::nilVal();
}

Value apiUiSetScale(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::nilVal();
    if (args.empty() || !args[0].isNumber()) return Value::nilVal();
    float s = (float)args[0].numberVal;
    if (s < 0.5f) s = 0.5f;
    if (s > 3.0f) s = 3.0f;
    applyScale(s);
    return Value::nilVal();
}

Value apiUiGetScale(const std::vector<Value>&) {
    return Value::number((double)gUiScale);
}

Value apiUiFontAddTtf(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::number(-1);
    if (args.size() < 2) return Value::number(-1);
    std::string path = args[0].toString();
    float sizePx = (float)args[1].numberVal;
    if (sizePx <= 1.0f) sizePx = 16.0f;
    auto resolved = std::filesystem::path(EngineBindings::resolveAssetPath(path)).lexically_normal();
    ImFont* f = ImGui::GetIO().Fonts->AddFontFromFileTTF(resolved.string().c_str(), sizePx);
    if (!f) return Value::number(-1);
    ImGui_ImplOpenGL2_DestroyFontsTexture();
    ImGui_ImplOpenGL2_CreateFontsTexture();
    gFonts.push_back(f);
    return Value::number((double)((int)gFonts.size() - 1));
}

Value apiUiFontSetDefault(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::nilVal();
    if (args.empty() || !args[0].isNumber()) return Value::nilVal();
    int id = (int)args[0].numberVal;
    if (id < 0) {
        ImGui::GetIO().FontDefault = nullptr;
        return Value::nilVal();
    }
    if (id < 0 || id >= (int)gFonts.size()) return Value::nilVal();
    ImGui::GetIO().FontDefault = gFonts[id];
    return Value::nilVal();
}

Value apiUiFontPush(const std::vector<Value>& args) {
    if (!uiEnabled()) return Value::nilVal();
    if (args.empty() || !args[0].isNumber()) return Value::nilVal();
    int id = (int)args[0].numberVal;
    if (id < 0 || id >= (int)gFonts.size()) return Value::nilVal();
    ImGui::PushFont(gFonts[id]);
    return Value::nilVal();
}

Value apiUiFontPop(const std::vector<Value>&) {
    if (!uiEnabled()) return Value::nilVal();
    ImGui::PopFont();
    return Value::nilVal();
}
}
