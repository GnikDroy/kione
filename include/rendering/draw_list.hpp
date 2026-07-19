#pragma once

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

namespace k2 {

struct DrawCommand {
    enum class Kind : std::uint8_t { Line, Rect, Circle, Point, Polygon };

    Kind kind {};
    glm::vec2 a {}; // line start / center / point position
    glm::vec2 b {}; // line end / rect size
    float radius {};
    float width { 1.0f }; // line width / outline thickness / point size
    glm::vec4 color { 1.0f, 1.0f, 1.0f, 1.0f };
    float z {};
    bool filled { true };
    bool unlit { true };
    bool closed { true };
    int segments {};
    std::vector<glm::vec2> points {}; // polygon only
};

struct DrawList {
    std::vector<DrawCommand> commands {};
    bool overflowed {};
};

}
