#pragma once
#include <glm/glm.hpp>

#include "core/resource_container.hpp"
#include "core/utils.hpp"

namespace k2 {

struct SpriteComponent {
    glm::vec4 color { 1.0f, 1.0f, 1.0f, 1.0f };
    k2::ResourceID texture {};
    k2::Rectf uv_rect { 0.0f, 0.0f, 1.0f, 1.0f }; // This is normalized / in percentage.
};
}
