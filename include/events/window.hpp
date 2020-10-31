#pragma once

namespace k2 {
class Window;

struct WindowCloseEvent {
    Window* window;
};

struct WindowResizeEvent {
    Window* window;
    int width, height;
};

struct WindowFamebufferResizeEvent {
    Window* window;
    int width, height;
};

struct WindowContentScaleChangeEvent {
    Window* window;
    float x, y;
};

struct WindowRepositionEvent {
    Window* window;
    int x, y;
};

struct WindowIconifyEvent {
    Window* window;
    bool iconified;
};

struct WindowMaximizeEvent {
    Window* window;
    bool maximized;
};

struct WindowFocusChangeEvent {
    Window* window;
    bool focused;
};
}  // namespace k2