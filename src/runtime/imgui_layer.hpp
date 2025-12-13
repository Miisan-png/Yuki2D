#pragma once
namespace yuki {
class Window;
class ImGuiLayer {
public:
    ImGuiLayer();
    ~ImGuiLayer();
    bool init(Window& window);
    void shutdown();
    void newFrame();
    void render();
    bool isEnabled() const;
};
}
