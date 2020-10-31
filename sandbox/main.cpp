#include <iostream>

#include "kione2D.hpp"

class Sandbox : public k2::App {
   public:
    k2::Window window;

    Sandbox() : App(), window{} {
        k2::Logger::app->info("Sandbox application started.");
    }

    virtual void run() {
        while (true) {
            window.update();
            k2::event_dispatcher.update();
        }
    }

    virtual ~Sandbox() {
        k2::Logger::app->info("Sandbox application stopped.");
    }
};

std::unique_ptr<k2::App> create_app() {
    return std::unique_ptr<k2::App>{std::make_unique<Sandbox>()};
}