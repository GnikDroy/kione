#include "kione2D.hpp"
#include "platform/entry_point.hpp"

#include "editor_layer.hpp"
#include "widgets/log_viewer.hpp"

#include <memory>

class Editor : public k2::App {
    k2::Window window { { .title { "Kione" } } };
    k2::EditorLayer editor_layer;
    bool running = true;

public:
    Editor()
        : editor_layer(window) {
        k2::Log::app().info("Editor application started.");
    }
    ~Editor() override { k2::Log::app().info("Editor application closed."); }

    void run() override {
        glEnable(GL_MULTISAMPLE);
        while (running) {
            window.update();
            handle_events();

            editor_layer.start();
            editor_layer.update(0.0f);

            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            editor_layer.render();
        }
    }

    void handle_events() {
        using namespace k2::literals;
        while (!window.events.empty()) {
            const auto event = std::move(window.events.front());
            window.events.pop();

            if (event->type == "WindowFramebufferResizeEvent"_fnv1a) {
                auto* e = reinterpret_cast<k2::WindowFramebufferResizeEvent*>(event.get());
                glViewport(0, 0, e->width, e->height);
            } else if (event->type == "WindowCloseEvent"_fnv1a) {
                k2::Log::app().info("Window Close Event Received.");
                running = false;
            }

            editor_layer.handle_event(event.get());
        }
    }
};

auto create_app() -> std::unique_ptr<k2::App> {
    k2::editor::EditorLoggerSink::get(); // Makes sure the sink is registered so that logs are tracked from the very
                                         // beginning.
    return std::make_unique<Editor>();
}
