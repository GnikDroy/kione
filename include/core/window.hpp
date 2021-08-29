#pragma once
#include <any>
#include <cstdint>
#include <functional>
#include <memory>
#include <queue>
#include <string>

#include "core/ecs.hpp"
#include "core/keyboard.hpp"
#include "core/mouse.hpp"

namespace k2 {
struct Event;

struct WindowConfig {
    std::string title { "Kione 2D" };
    std::uint32_t x_pos {}, y_pos {}, width = 1280, height = 720;
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

    explicit Window(const WindowConfig& = {});
    ~Window();

    void update();

    [[nodiscard]] ::uint32_t get_width() const;
    [[nodiscard]] ::uint32_t get_height() const;

    [[nodiscard]] void* get_native_handle() const;
    [[nodiscard]] void* get_native_display() const;

    void set_vsync(bool status);
    [[nodiscard]] bool is_vsync() const;

    std::queue<std::unique_ptr<Event>> events;
};
} // namespace k2