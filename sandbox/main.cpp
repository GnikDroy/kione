#include <iostream>

#include "bgfx/bgfx.h"
#include "core/rendering/shader.hpp"
#include "kione2D.hpp"

#include "core/imgui_layer.hpp"

#include <map>
#include <ranges>

struct PosColorVertex {
    float x;
    float y;
    float z;
    uint32_t abgr;
};

static PosColorVertex cubeVertices[] = {
    {-1.0f,  1.0f,  1.0f, 0xff000000 },
    { 1.0f,  1.0f,  1.0f, 0xff0000ff },
    {-1.0f, -1.0f,  1.0f, 0xff00ff00 },
    { 1.0f, -1.0f,  1.0f, 0xff00ffff },
    {-1.0f,  1.0f, -1.0f, 0xffff0000 },
    { 1.0f,  1.0f, -1.0f, 0xffff00ff },
    {-1.0f, -1.0f, -1.0f, 0xffffff00 },
    { 1.0f, -1.0f, -1.0f, 0xffffffff },
};

static const uint16_t cubeTriList[] =
{
    0, 1, 2,
    1, 3, 2,
    4, 6, 5,
    5, 6, 7,
    0, 2, 4,
    4, 2, 6,
    1, 5, 3,
    5, 7, 3,
    0, 4, 1,
    4, 5, 1,
    2, 3, 6,
    6, 3, 7,
};

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

        auto bg_layout = bgfx::VertexLayout();

        bg_layout.begin()
                 .add(bgfx::Attrib::Position, 3,bgfx::AttribType::Float)
                 .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
                 .end();

        auto vertex_layout = k2::VertexLayout{bg_layout};

        auto vertex_buffer = k2::VertexBuffer(bgfx::makeRef(cubeVertices, sizeof(cubeVertices)),bg_layout);
        auto index_buffer = k2::IndexBuffer(bgfx::makeRef(cubeTriList, sizeof(cubeTriList)));

        auto vertex_shader = k2::Shader::load("v_cube.bin");
        auto fragment_shader = k2::Shader::load("f_cube.bin");

        if (!vertex_shader.has_value() || !fragment_shader.has_value()) {
            k2::Logger::app->critical("Cannot locate shader");
        } else {
            k2::Logger::app->info("Shaders loaded successfully.");
        }
        auto program = k2::Program{vertex_shader.value(), fragment_shader.value()};


        while (running) {

            // Populate event buffer.
            window.update();

            // Handle all events in layers.
            for (; !window.events.empty(); window.events.pop()) {
                const auto event = std::move(window.events.front());

                if (event->type == "WindowFramebufferResizeEvent"_fnv1a) {

                }
                if (event->type == "WindowCloseEvent"_fnv1a) {
                    k2::Logger::app->info("Window Close Event Received.");
                    running = false;
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
        k2::Logger::app->info("Sandbox application run() end.");
    }

    ~Sandbox() override {
        k2::Logger::app->info("Sandbox application stopped.");
    }
};

std::unique_ptr<k2::App> create_app() {
    return std::unique_ptr<k2::App>{std::make_unique<Sandbox>()};
}
