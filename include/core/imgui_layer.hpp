#pragma once

#include "core/imgui_theme.hpp"
#include "layer.hpp"
#include <unordered_set>

namespace k2 {
class Window;

class ImguiLayer : public Layer {
    static inline std::unordered_set<k2::Window*> initialized_windows {};
    k2::Window* window;

protected:
    std::unique_ptr<Imgui::ImGuiTheme> theme;

public:
    ImguiLayer(k2::Window& win, std::unique_ptr<Imgui::ImGuiTheme> theme = std::make_unique<Imgui::ImGuiThemeDark>());

    ImguiLayer(const ImguiLayer&) = delete;
    ImguiLayer& operator=(const ImguiLayer&) = delete;

    ~ImguiLayer() override;
    void update(float) override;
    bool handle_event(const Event*) override;
    void start();
    void render() override;
};
}