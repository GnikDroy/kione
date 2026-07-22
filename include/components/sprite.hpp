#pragma once
#include <cstdint>

#include <glm/glm.hpp>

#include "asset/asset_handle.hpp"
#include "core/utils.hpp"

namespace k2 {

enum class BlendMode : std::uint8_t { Alpha, Additive };

struct SpriteComponent {
    glm::vec4 color { 1.0f, 1.0f, 1.0f, 1.0f };
    k2::AssetHandle texture {};
    k2::Rectf region { .x = 0.0f, .y = 0.0f, .w = 64.0f, .h = 64.0f };
    glm::vec2 size { 64.0f, 64.0f };
    bool unlit { false };
    BlendMode blend { BlendMode::Alpha };
    float intensity { 1.0f };
};
}
