#include <iostream>

#include "bgfx/bgfx.h"
#include "kione2D.hpp"

#include "core/imgui_layer.hpp"

#include <map>

class Sandbox : public k2::App {
public:
    k2::Window window;
    bool running = true;
    std::map<std::uint64_t, std::unique_ptr<k2::Layer>> layers;

    Sandbox() : App(), window{} {
        k2::Logger::app->info("Sandbox application started.");

        using namespace k2::literals;
        window.set_event_handler([this](auto event) {
            if (event.type == "WindowCloseEvent"_fnv1a) {
                running = false;
            }
        });
    }

    void run() override {
        activate_renderer(window);

        using namespace k2::literals;
        layers["imgui_layer"_fnv1a] = std::make_unique<k2::ImguiLayer>(window);

        while (running) {

            // Populate event buffer.
            window.update();

            // Handle all events in layers.

            // Render
            bgfx::setViewRect(0, 0, 0, uint16_t(window.get_width()), uint16_t(window.get_height()));
            bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0xffffffff, 1.0f, 0);
            for (auto&[id, layer]: layers) { layer->render(); }
            bgfx::touch(0);
            bgfx::frame();

            // Clear event buffer.
        }
    }

    ~Sandbox() override {
        k2::Logger::app->info("Sandbox application stopped.");
    }
};

std::unique_ptr<k2::App> create_app() {
    return std::unique_ptr<k2::App>{std::make_unique<Sandbox>()};
}
