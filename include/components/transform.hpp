#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace k2 {

struct TransformComponent {
    glm::vec3 translation;
    glm::quat orientation;
    glm::vec3 scale{1.0f, 1.0f, 1.0f};

    [[nodiscard]] glm::mat4 get_matrix() const {
        return glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(orientation)
            * glm::scale(glm::mat4(1.0f), scale);
    }
};

}
