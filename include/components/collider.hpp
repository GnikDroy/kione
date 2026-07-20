#pragma once

#include <cstdint>
#include <variant>

#include <glm/glm.hpp>

namespace k2 {

struct BoxShape {
    glm::vec2 size { 32.0f, 32.0f };
};

struct CircleShape {
    float radius { 16.0f };
};

struct PillShape {
    float radius { 16.0f };
    float half_height { 16.0f };
};

struct ColliderComponent {
    std::variant<BoxShape, CircleShape, PillShape> shape { CircleShape {} };
    std::uint32_t layer { 1 };
    std::uint32_t mask { 0xffffffff };
};

}
