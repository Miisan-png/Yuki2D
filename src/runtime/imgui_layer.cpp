#include "imgui_layer.hpp"
#include "../core/window.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>

namespace yuki {
ImGuiLayer::ImGuiLayer() {}
ImGuiLayer::~ImGuiLayer() { shutdown(); }

bool ImGuiLayer::init(Window& window) {
    if (isEnabled()) return true;
    if (!window.getNativeWindow()) return false;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;
    io.ConfigErrorRecovery = true;
    io.ConfigErrorRecoveryEnableAssert = false;
    io.ConfigErrorRecoveryEnableDebugLog = true;
    io.ConfigErrorRecoveryEnableTooltip = true;
    ImGui::StyleColorsDark();
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
    ImGui_ImplGlfw_InitForOpenGL(window.getNativeWindow(), true);
    ImGui_ImplOpenGL2_Init();
    return true;
}

void ImGuiLayer::shutdown() {
    if (!isEnabled()) return;
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::newFrame() {
    if (!isEnabled()) return;
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::render() {
    if (!isEnabled()) return;
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

bool ImGuiLayer::isEnabled() const {
    return ImGui::GetCurrentContext() != nullptr;
}
}
