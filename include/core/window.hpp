#pragma once
#include <string>
#include <cstdint>

namespace k2
{
    struct WindowConfig
    {
        std::string window_title;
        int window_x, window_y, window_width, window_height;
        std::uint32_t window_flags;
    };
    
    struct Window;
    struct Renderer;
} // namespace k2