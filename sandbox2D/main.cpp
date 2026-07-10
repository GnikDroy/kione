#include <ranges>

#include "kione2D.hpp"
#include "platform/entry_point.hpp"

#include "core/imgui_layer.hpp"
#include "rendering/debug.hpp"
#include "scene_layer.hpp"

class Sandbox2D : public k2::App {
public:
    k2::Window window;
    bool running = true;
    std::vector<std::unique_ptr<k2::Layer>> layers;

    Sandbox2D() {
        k2::Log::app().info("Sandbox2D application started.");
        k2::enable_debug();
    }

    void handle_events() {
        using namespace k2::literals;
        while (!window.events.empty()) {
            const auto ev = std::move(window.events.front());
            window.events.pop();

            HANDLE_EVENT(k2::WindowFramebufferResizeEvent, ev.get(), event, {
                auto width = event.width;
                auto height = event.height;
                glViewport(0, 0, width, height);
            })

            HANDLE_EVENT(k2::WindowCloseEvent, ev.get(), event, {
                k2::Log::app().info("Window Close Event Received.");
                running = false;
            })

            for (auto& layer : std::views::reverse(layers)) {
                if (layer->handle_event(ev.get())) {
                    break;
                }
            }
        }
    }

    void run() override {
        using namespace std::chrono;

        layers.push_back(std::make_unique<SceneLayer>(window));
        layers.push_back(std::make_unique<k2::ImguiLayer>(window));
        auto* imgui_layer = static_cast<k2::ImguiLayer*>(layers[1].get());

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);

        auto last_frame = steady_clock::now();

        while (running) {
            auto current_frame = steady_clock::now();
            auto dt = duration<float>(current_frame - last_frame).count();
            last_frame = current_frame;

            // Update and handle events
            window.update();
            for (auto& layer : layers) {
                layer->update(dt);
            }
            handle_events();

            // Clear Screen
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            // Render
            imgui_layer->start();
            for (auto& layer : layers) {
                layer->render();
            }
        }
    }

    ~Sandbox2D() override { k2::Log::app().info("Sandbox2D application stopped."); }
};

auto create_app() -> std::unique_ptr<k2::App> { return std::make_unique<Sandbox2D>(); }
