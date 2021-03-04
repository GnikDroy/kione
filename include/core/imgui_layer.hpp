#pragma once

#include "layer.hpp"
#include <unordered_set>

namespace k2 {
    class Window;

    class ImguiLayer : public Layer {
        static inline std::unordered_set<k2::Window*> initialized_windows{};
        k2::Window* window;
    public:
        ImguiLayer(k2::Window& win);
        ~ImguiLayer() override;
        void update() override;
    };
}