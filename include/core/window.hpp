#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <any>

#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ecs.hpp"

namespace k2 {
struct Event;

struct WindowConfig {
    std::string title{"Kione 2D"};
    std::uint32_t x_pos{}, y_pos{}, width = 1280, height = 720;
    std::function<void(Event)> event_handler;
};

class Window {
    struct Impl;
    std::unique_ptr<Impl> impl;

   public:
    friend class KeyboardDevice;
    KeyboardDevice keyboard;
    
    friend class MouseDevice;
    MouseDevice mouse;

    friend class ImguiLayer;
    class ImguiLayer;

    Window(const WindowConfig & = {});
    ~Window();

    void update();

    std::uint32_t get_width() const;
    std::uint32_t get_height() const;

    void set_event_handler(std::function<void(Event)>);

    void* get_native_handle() const;
    void* get_native_display() const;
    
    void set_vsync(bool status);
    bool is_vsync() const;

};
}  // namespace k2