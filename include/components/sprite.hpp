#pragma once
#include <glm/glm.hpp>

#include "asset/asset_handle.hpp"
#include "core/utils.hpp"

namespace k2 {

struct SpriteComponent {
    glm::vec4 color { 1.0f, 1.0f, 1.0f, 1.0f };
    k2::AssetHandle texture {};
    k2::Rectf uv_rect { .x = 0.0f, .y = 0.0f, .w = 1.0f, .h = 1.0f };
    bool unlit { false };
};
}
