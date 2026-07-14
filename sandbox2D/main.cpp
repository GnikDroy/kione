#include "kione2D.hpp"
#include "platform/entry_point.hpp"

#include "core/imgui_layer.hpp"
#include "rendering/debug.hpp"
#include "scene_layer.hpp"

class Sandbox2D : public k2::App {
public:
    Sandbox2D() {
        k2::Log::app().info("Sandbox2D application started.");
        k2::enable_debug();

        layers.push_back(std::make_unique<SceneLayer>(window));
        layers.push_back(std::make_unique<k2::ImguiLayer>(window));

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
    }

    ~Sandbox2D() override { k2::Log::app().info("Sandbox2D application stopped."); }
};

auto create_app(std::vector<std::string>) -> std::unique_ptr<k2::App> { return std::make_unique<Sandbox2D>(); }
