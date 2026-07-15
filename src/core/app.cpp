#include "core/app.hpp"

#include <algorithm>
#include <chrono>
#include <ranges>

#include <glad/glad.h>

#include "events/event.hpp"
#include "events/window.hpp"

namespace k2 {
App::App(const WindowConfig& config)
    : window { config } { }

App::~App() = default;

void App::close() { running = false; }

void App::run() {
    using namespace std::chrono;
    auto last_frame = steady_clock::now();

    while (running) {
        window.poll();
        dispatch_events();

        auto current_frame = steady_clock::now();
        auto dt = std::min(duration<float>(current_frame - last_frame).count(), max_dt);
        last_frame = current_frame;

        for (auto& layer : layers) {
            layer->begin_frame();
        }
        for (auto& layer : layers) {
            layer->update(dt);
        }

        glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        for (auto& layer : layers) {
            layer->render();
        }

        window.swap();
    }
}

void App::dispatch_events() {
    while (!window.events.empty()) {
        const auto event = std::move(window.events.front());
        window.events.pop();

        HANDLE_EVENT(WindowCloseEvent, event.get(), close_event, { close(); })
        HANDLE_EVENT(WindowFramebufferResizeEvent, event.get(), resize_event,
            { glViewport(0, 0, resize_event.width, resize_event.height); })

        for (auto& layer : std::views::reverse(layers)) {
            if (layer->handle_event(event.get())) {
                break;
            }
        }
    }
}
} // namespace k2
