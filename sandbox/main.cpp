#include "kione2D.hpp"
#include <iostream>

class Sandbox : public k2::App
{
public:
    Sandbox() : App() {}
    virtual void run()
    {
        k2::Logger::app->info("Sandbox application started.");
    }
    virtual ~Sandbox() {
        k2::Logger::app->info("Sandbox application stopped.");
    }
};

std::unique_ptr<k2::App> create_app() { return std::unique_ptr<k2::App>{std::make_unique<Sandbox>()}; }