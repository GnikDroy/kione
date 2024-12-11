#pragma once

#include "events/event.hpp"

namespace k2 {
using namespace k2::literals;
struct WindowCloseEvent : public Event {
    constexpr static inline std::uint64_t hash = "WindowCloseEvent"_fnv1a;
    WindowCloseEvent()
        : Event(hash) { }
};

struct WindowResizeEvent : public Event {
    constexpr static inline std::uint64_t hash = "WindowResizeEvent"_fnv1a;
    WindowResizeEvent()
        : Event(hash) {};
    int width {}, height {};
};

struct WindowFramebufferResizeEvent : public Event {
    constexpr static inline std::uint64_t hash = "WindowFramebufferResizeEvent"_fnv1a;
    WindowFramebufferResizeEvent()
        : Event(hash) { }
    int width {}, height {};
};

struct WindowContentScaleChangeEvent : public Event {
    constexpr static inline std::uint64_t hash = "WindowContentScaleChangeEvent"_fnv1a;
    WindowContentScaleChangeEvent()
        : Event(hash) { }
    float x {}, y {};
};

struct WindowRepositionEvent : public Event {
    constexpr static inline std::uint64_t hash = "WindowRepositionEvent"_fnv1a;
    WindowRepositionEvent()
        : Event(hash) { }
    int x {}, y {};
};

struct WindowIconifyEvent : public Event {
    constexpr static inline std::uint64_t hash = "WindowIconifyEvent"_fnv1a;
    WindowIconifyEvent()
        : Event(hash) { }
    bool iconified {};
};

struct WindowMaximizeEvent : public Event {
    constexpr static inline std::uint64_t hash = "WindowMaximizeEvent"_fnv1a;
    WindowMaximizeEvent()
        : Event(hash) { }
    bool maximized {};
};

struct WindowFocusChangeEvent : public Event {
    constexpr static inline std::uint64_t hash = "WindowFocusChangeEvent"_fnv1a;
    WindowFocusChangeEvent()
        : Event(hash) { }
    bool focused {};
};
} // namespace k2
