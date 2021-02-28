#include <iostream>

#include "bgfx/bgfx.h"
#include "kione2D.hpp"

class Sandbox : public k2::App {
   public:
    k2::Window window;
    bool running = true;

    Sandbox() : App(), window{} {
        k2::Logger::app->info("Sandbox application started.");
        window.event_dispatcher.sink<k2::WindowCloseEvent>()
            .connect<&Sandbox::close>(this);
    }

    void close(k2::WindowCloseEvent) {
        k2::Logger::app->info("Window Close Event received");
        running = false;
    }

    void run() override {
        activate_renderer(window);
        
        bgfx::setDebug(BGFX_DEBUG_STATS);
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0xffffffff, 1.0f, 0);

        while (running) {
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
