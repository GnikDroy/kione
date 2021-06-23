#include <map>
#include <ranges>

#include "kione2D.hpp"

#include "glad/glad.h"

#include "core/imgui_layer.hpp"
#include "scene_layer.hpp"


class Sandbox2D : public k2::App {
public:
    k2::Window window;
    bool running = true;
    std::vector<std::unique_ptr<k2::Layer>> layers;

    Sandbox2D() : App(), window{} {
        k2::Logger::app->info("Sandbox2D application started.");
    }

    void handle_events() {
        using namespace k2::literals;
        while (!window.events.empty()) {
            const auto event = std::move(window.events.front());
            window.events.pop();

            // Handle all basic events
            if (event->type == "WindowFramebufferResizeEvent"_fnv1a) {
                auto e = reinterpret_cast<k2::WindowFramebufferResizeEvent *>(event.get());
                glViewport(0, 0, e->width, e->height);
            } else if (event->type == "WindowCloseEvent"_fnv1a) {
                k2::Logger::app->info("Window Close Event Received.");
                running = false;
            }

            for (auto &layer: std::views::reverse(layers)) {
                if (layer->handle_event(event.get())) { break; }
            }
        }
    }

    void run() override {
        using namespace std::chrono;

        layers.push_back(std::make_unique<SceneLayer>(window));
        layers.push_back(std::make_unique<k2::ImguiLayer>(window));
        auto imgui_layer = reinterpret_cast<k2::ImguiLayer*>(layers[1].get());

        glEnable(GL_DEPTH_TEST);

        auto current_frame = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        auto last_frame = current_frame;

        while (running) {
            current_frame = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            auto dt = float(current_frame - last_frame) / 1000.0f;
            last_frame = current_frame;


            //Update and handle events
            window.update();
            for (auto &layer: layers) { layer->update(dt); }
            handle_events();

            // Clear Screen
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Render
            imgui_layer->start();
            for (auto &layer: layers) { layer->render(); }
        }
    }

    ~Sandbox2D() override {
        k2::Logger::app->info("Sandbox2D application stopped.");
    }
};

std::unique_ptr<k2::App> create_app() {
    return std::unique_ptr<k2::App>{std::make_unique<Sandbox2D>()};
}