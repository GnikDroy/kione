#pragma once

#include <glm/glm.hpp>

namespace k2 {

struct Environment {
    glm::vec3 ambient_color { 1.0f, 1.0f, 1.0f };
    float ambient_intensity { 1.0f };
    glm::vec4 clear_color { 0.0f, 0.0f, 0.0f, 1.0f };
    bool bloom { true };
    float bloom_intensity { 1.0f };
    float bloom_threshold { 1.0f };
};

}
