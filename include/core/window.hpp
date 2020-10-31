#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include "core/keyboard.hpp"
#include "core/mouse.hpp"

namespace k2 {
struct WindowConfig {
    std::string title{"Kione 2D"};
    std::uint32_t x_pos{}, y_pos{}, width = 1280, height = 720;
};

class Window {
    class Impl;
    std::unique_ptr<Impl> impl;

   public:
    friend class KeyboardDevice;
    KeyboardDevice keyboard;
    
    friend class MouseDevice;
    MouseDevice mouse;

    Window(const WindowConfig & = {});
    ~Window();

    void update();

    std::uint32_t get_width() const;
    std::uint32_t get_height() const;

    void set_vsync(bool status);
    bool is_vsync() const;
};
}  // namespace k2