#include "kione2D.hpp"
#include "platform/entry_point.hpp"

#include "core/imgui_layer.hpp"
#include "scene_layer.hpp"

class Sandbox : public k2::App {
public:
    Sandbox() {
        k2::Log::app().info("Sandbox application started.");

        layers.push_back(std::make_unique<SceneLayer>(window));
        layers.push_back(std::make_unique<k2::ImguiLayer>(window));

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }

    ~Sandbox() override { k2::Log::app().info("Sandbox application stopped."); }
};

auto create_app() -> std::unique_ptr<k2::App> { return std::make_unique<Sandbox>(); }
