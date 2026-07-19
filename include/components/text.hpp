#pragma once
#include <string>

#include <glm/glm.hpp>

#include "asset/asset_handle.hpp"

namespace k2 {

struct TextComponent {
    k2::AssetHandle font {};
    std::string text {};
    float size { 32.0f };
    glm::vec4 color { 1.0f, 1.0f, 1.0f, 1.0f };
};
}
