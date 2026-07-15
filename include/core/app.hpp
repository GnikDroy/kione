#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "core/layer.hpp"
#include "core/window.hpp"

namespace k2 {
class App {
public:
    explicit App(const WindowConfig& config = {});
    virtual ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    void run();
    void close();

    Window window;

    float max_dt = 0.25f;

protected:
    std::vector<std::unique_ptr<Layer>> layers;
    glm::vec4 clear_color { 0.2f, 0.2f, 0.2f, 1.0f };

private:
    void dispatch_events();

    bool running = true;
};
} // namespace k2
