#include "kione2D.hpp"
#include "platform/entry_point.hpp"

#include "editor_layer.hpp"
#include "ui/widgets/log_viewer.hpp"

#include <format>
#include <memory>

class Editor : public k2::App {
public:
    explicit Editor(std::vector<std::string> args)
        : k2::App({ .title { "Kione Editor" }, .maximized = true }) {
        auto layer = std::make_unique<k2::EditorLayer>(window);
        if (!args.empty()) {
            if (auto opened = layer->open_project(args.front()); !opened) {
                k2::Log::app().error(std::format("Cannot open project '{}': {}", args.front(), opened.error()));
            }
        }
        layers.push_back(std::move(layer));
        glEnable(GL_MULTISAMPLE);
        k2::Log::app().info("Editor application started.");
    }

    ~Editor() override { k2::Log::app().info("Editor application closed."); }
};

auto create_app(std::vector<std::string> args) -> std::unique_ptr<k2::App> {
    k2::editor::EditorLoggerSink::get(); // Makes sure the sink is registered so that logs are tracked from the very
                                         // beginning.
    return std::make_unique<Editor>(std::move(args));
}
