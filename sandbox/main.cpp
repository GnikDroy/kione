#include <iostream>

#include "bgfx/bgfx.h"
#include "kione2D.hpp"

#include "core/imgui_layer.hpp"

#include <map>
#include <ranges>

class Sandbox : public k2::App {
public:
    k2::Window window;
    bool running = true;
    std::vector<std::unique_ptr<k2::Layer>> layers;

    Sandbox() : App(), window{} {
        k2::Logger::app->info("Sandbox application started.");
    }

    void run() override {
        activate_renderer(window);

        using namespace k2::literals;
        layers.push_back(std::make_unique<k2::ImguiLayer>(window));

        auto view_width = (uint16_t) window.get_width();
        auto view_height = (uint16_t) window.get_height();
        while (running) {

            // Populate event buffer.
            window.update();

            // Handle all events in layers.
            for (; !window.events.empty(); window.events.pop()) {
                const auto event = std::move(window.events.front());

                if (event->type == "WindowFramebufferResizeEvent"_fnv1a) {

                }
                if (event->type == "WindowCloseEvent"_fnv1a) {
                    running = false;
                    break;
                }

                for (auto &layer: std::views::reverse(layers)) {
                    if (layer->handle_event(event.get())) { break; }
                }
            }

            // Render
            bgfx::setViewRect(0, 0, 0, view_width, view_height);
            bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0xffffffff, 1.0f, 0);
            for (auto &layer: layers) { layer->render(); }
            bgfx::touch(0);
            bgfx::frame();

        }
    }

    ~Sandbox() override {
        k2::Logger::app->info("Sandbox application stopped.");
    }
};

std::unique_ptr<k2::App> create_app() {
    return std::unique_ptr<k2::App>{std::make_unique<Sandbox>()};
}
