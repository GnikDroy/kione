#pragma once

#include "layer.hpp"
#include <unordered_set>

namespace k2 {
    class Window;

    class ImguiLayer : public Layer {
        static inline std::unordered_set<k2::Window*> initialized_windows{};
        k2::Window* window;
    public:
        explicit ImguiLayer(k2::Window& win);

        ImguiLayer(const ImguiLayer&) = delete;
        ImguiLayer& operator=(const ImguiLayer&) = delete;

        ~ImguiLayer() override;
        void update() override;
        bool handle_event(const Event*) override;
        void render() override;
    };
}