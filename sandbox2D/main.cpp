#include "kione2D.hpp"
#include "platform/entry_point.hpp"

#include "core/imgui_layer.hpp"
#include "rendering/debug.hpp"
#include "scene_layer.hpp"

class Sandbox2D : public k2::App {
public:
    explicit Sandbox2D(const std::string& project_path) {
        k2::Log::app().info("Sandbox2D application started.");
        k2::enable_debug();

        layers.push_back(std::make_unique<SceneLayer>(window, project_path));
        layers.push_back(std::make_unique<k2::ImguiLayer>(window));

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
    }

    ~Sandbox2D() override { k2::Log::app().info("Sandbox2D application stopped."); }
};

auto create_app(std::vector<std::string> args) -> std::unique_ptr<k2::App> {
    return std::make_unique<Sandbox2D>(args.empty() ? "res/project.k2project" : args.front());
}
