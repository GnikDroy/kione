#pragma once
#include <glm/glm.hpp>

#include "asset/asset_handle.hpp"

namespace k2 {

struct AmbientLight {
    glm::vec3 color { 1.0f, 1.0f, 1.0f };
    float intensity { 1.0f };
};

struct PointLight {
    glm::vec3 color { 1.0f, 1.0f, 1.0f };
    float intensity { 1.0f };
    float radius { 500.0f };
};

struct SpotLight {
    glm::vec3 color { 1.0f, 1.0f, 1.0f };
    float intensity { 1.0f };
    float radius { 500.0f };
    float inner_angle { 0.3f };
    float outer_angle { 0.6f };
};

struct SpriteLight {
    AssetHandle texture {};
    glm::vec3 color { 1.0f, 1.0f, 1.0f };
    float intensity { 1.0f };
};

}
