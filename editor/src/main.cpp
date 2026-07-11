#include "kione2D.hpp"
#include "platform/entry_point.hpp"

#include "editor_layer.hpp"
#include "ui/widgets/log_viewer.hpp"

#include <memory>

class Editor : public k2::App {
public:
    Editor()
        : k2::App({ .title { "Kione Editor" } }) {
        layers.push_back(std::make_unique<k2::EditorLayer>(window));
        glEnable(GL_MULTISAMPLE);
        k2::Log::app().info("Editor application started.");
    }

    ~Editor() override { k2::Log::app().info("Editor application closed."); }
};

auto create_app() -> std::unique_ptr<k2::App> {
    k2::editor::EditorLoggerSink::get(); // Makes sure the sink is registered so that logs are tracked from the very
                                         // beginning.
    return std::make_unique<Editor>();
}
