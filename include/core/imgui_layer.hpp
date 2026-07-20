#pragma once

#include "core/imgui_theme.hpp"
#include "layer.hpp"

struct ImFont;

namespace k2 {
class Window;

class ImguiLayer : public Layer {
    static inline bool initialized { false };

protected:
    k2::Window* window;

public:
    std::unique_ptr<Imgui::ImGuiTheme> theme;
    ImFont* icon_font {};

    explicit ImguiLayer(
        k2::Window& win, std::unique_ptr<Imgui::ImGuiTheme> theme = std::make_unique<Imgui::ImGuiThemeDark>());

    ImguiLayer(const ImguiLayer&) = delete;
    ImguiLayer& operator=(const ImguiLayer&) = delete;

    ~ImguiLayer() override;
    void begin_frame() override;
    void update(float) override;
    bool handle_event(const Event*) override;
    void render() override;
};
}
