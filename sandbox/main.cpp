#include <iostream>

#include "bgfx/bgfx.h"
#include "kione2D.hpp"
#include "core/imgui_layer.hpp"

class Sandbox : public k2::App {
public:
    k2::Window window;
    bool running = true;
    std::vector<k2::Layer> layers;

    Sandbox() : App(), window{} {
        k2::Logger::app->info("Sandbox application started.");

        using namespace k2::literals;
        window.set_event_handler([this](auto event){
           if (event.type == "WindowCloseEvent"_fnv1a){
              running = false;
           }
        });
    }

    void close(k2::WindowCloseEvent) {
        k2::Logger::app->info("Window Close Event received");
        running = false;
    }

    void run() override {
        activate_renderer(window);


        k2::ImguiLayer imgui_layer{window};
        while (running) {
            bgfx::setViewRect(0, 0, 0, uint16_t(window.get_width()), uint16_t(window.get_height()) );
            bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0xffffffff, 1.0f, 0);

            imgui_layer.update();
            bgfx::touch(0);
            bgfx::frame();

            window.update();
        }
    }

    ~Sandbox() override {
        k2::Logger::app->info("Sandbox application stopped.");
    }
};

std::unique_ptr<k2::App> create_app() {
    return std::unique_ptr<k2::App>{std::make_unique<Sandbox>()};
}
