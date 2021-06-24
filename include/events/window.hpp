#pragma once

#include "events/event.hpp"

namespace k2 {
using namespace k2::literals;
struct WindowCloseEvent : public Event {
    WindowCloseEvent()
        : Event("WindowCloseEvent"_fnv1a) { }
};

struct WindowResizeEvent : public Event {
    WindowResizeEvent()
        : Event("WindowResizeEvent"_fnv1a) {};
    int width {}, height {};
};

struct WindowFramebufferResizeEvent : public Event {
    WindowFramebufferResizeEvent()
        : Event("WindowFramebufferResizeEvent"_fnv1a) { }
    int width {}, height {};
};

struct WindowContentScaleChangeEvent : public Event {
    WindowContentScaleChangeEvent()
        : Event("WindowContentScaleChangeEvent"_fnv1a) { }
    float x {}, y {};
};

struct WindowRepositionEvent : public Event {
    WindowRepositionEvent()
        : Event("WindowRepositionEvent"_fnv1a) { }
    int x {}, y {};
};

struct WindowIconifyEvent : public Event {
    WindowIconifyEvent()
        : Event("WindowIconifyEvent"_fnv1a) { }
    bool iconified {};
};

struct WindowMaximizeEvent : public Event {
    WindowMaximizeEvent()
        : Event("WindowMaximizeEvent"_fnv1a) { }
    bool maximized {};
};

struct WindowFocusChangeEvent : public Event {
    WindowFocusChangeEvent()
        : Event("WindowFocusChangeEvent"_fnv1a) { }
    bool focused {};
};
} // namespace k2