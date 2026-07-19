#pragma once

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

#include "components/sprite.hpp"
#include "core/resource_container.hpp"

namespace k2 {

struct Vertex2D {
    glm::vec3 position;
    glm::vec4 color;
    glm::vec2 texture_coordinate;
    ResourceID texture;
};

struct Drawable {
    float z {};
    glm::mat4 transform { 1.0f };
    std::vector<Vertex2D> vertices {};
    std::vector<std::uint32_t> indices {};
    bool unlit {};
    BlendMode blend { BlendMode::Alpha };
};

}
