#include <iostream>

#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include "kione2D.hpp"

class Sandbox : public k2::App {
   public:
    k2::Window window;
    bool running = true;

    Sandbox() : App(), window{} {
        k2::Logger::app->info("Sandbox application started.");
        k2::event_dispatcher.sink<k2::WindowCloseEvent>()
            .connect<&Sandbox::close>(this);
    }

    void close(k2::WindowCloseEvent) {
        k2::Logger::app->info("Window Close Event received");
        running = false;
    }

    virtual void run() override {
        activate_renderer(window);
        
        bgfx::setDebug(BGFX_DEBUG_STATS);
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0xffffffff, 1.0f, 0);

        while (running) {
            // This dummy draw call is here to make sure that view 0 is cleared
            // if no other draw calls are submitted to view 0.
            bgfx::touch(0);

            // Use debug font to print information about this example.
            bgfx::dbgTextClear();
            bgfx::dbgTextPrintf(0, 1, 0x4f, "bgfxTemplate");
            bgfx::dbgTextPrintf(0, 2, 0x6f, "Description: Minimal bgfx + GLFW application.");
            bgfx::dbgTextPrintf(0, 4, 0x4f, "Press F1 to toggle bgfx stats, Esc to quit");
            bgfx::frame();
            window.update();
            k2::event_dispatcher.update();
        }
    }

    virtual ~Sandbox() override {
        k2::Logger::app->info("Sandbox application stopped.");
    }
};

std::unique_ptr<k2::App> create_app() {
    return std::unique_ptr<k2::App>{std::make_unique<Sandbox>()};
}
