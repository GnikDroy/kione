#pragma once
#include <algorithm>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "components/relation.hpp"
#include "core/logger.hpp"

namespace k2 {

struct TransformComponent {
    glm::vec3 translation {};
    glm::quat orientation { 1.0f, 0.0f, 0.0f, 0.0f };
    glm::vec3 scale { 1.0f, 1.0f, 1.0f };

    [[nodiscard]] glm::mat4 get_matrix() const {
        return glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(orientation)
            * glm::scale(glm::mat4(1.0f), scale);
    }

    void set_from_matrix(const glm::mat4& matrix) {
        translation = glm::vec3(matrix[3]);

        scale = { glm::length(glm::vec3(matrix[0])), glm::length(glm::vec3(matrix[1])),
            glm::length(glm::vec3(matrix[2])) };

        constexpr auto epsilon = 1e-8f;
        glm::mat3 rotation { glm::vec3(matrix[0]) / std::max(scale.x, epsilon),
            glm::vec3(matrix[1]) / std::max(scale.y, epsilon), glm::vec3(matrix[2]) / std::max(scale.z, epsilon) };
        orientation = glm::quat_cast(rotation);
    }

    // Transforms are local.
    template <class EntityType>
    static glm::mat4 world(const entt::basic_registry<EntityType>& registry, EntityType entity) {
        glm::mat4 result { 1.0f };
        // Depth guard: hierarchy cycles would otherwise loop forever.
        size_t depth = 0;
        const size_t MAX_DEPTH = 256;
        auto current = entity;
        for (; current != entt::null && depth < MAX_DEPTH; depth++) {
            if (const auto* transform = registry.template try_get<TransformComponent>(current)) {
                result = transform->get_matrix() * result;
            }
            const auto* relation = registry.template try_get<RelationComponent>(current);
            current = relation ? relation->parent : EntityType { entt::null };
        }
        if (current != entt::null) {
            static bool warned = false;
            if (!warned) {
                warned = true;
                Log::core().warn("TransformComponent::world truncated a parent chain deeper than 256 entities");
            }
        }
        return result;
    }

    template <class EntityType>
    static glm::mat4 parent_world(const entt::basic_registry<EntityType>& registry, EntityType entity) {
        const auto* relation = registry.template try_get<RelationComponent>(entity);
        if (relation == nullptr || relation->parent == entt::null) {
            return glm::mat4 { 1.0f };
        }
        return world(registry, relation->parent);
    }
};

}
