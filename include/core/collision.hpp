#pragma once

#include <vector>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "components/collider.hpp"
#include "components/transform.hpp"

namespace k2::collision {

struct WorldCollider {
    ColliderComponent collider {};
    glm::vec2 center {};
    float angle {};
};

WorldCollider world_collider(const ColliderComponent& collider, const TransformComponent& transform);

bool overlaps_maskless(const WorldCollider& a, const WorldCollider& b);

bool overlaps(entt::registry& registry, entt::entity a, entt::entity b);

std::vector<entt::entity> query_circle(
    entt::registry& registry, glm::vec2 center, float radius, std::uint32_t mask = 0xffffffff);

std::vector<entt::entity> query_aabb(
    entt::registry& registry, glm::vec2 center, glm::vec2 half_extents, std::uint32_t mask = 0xffffffff);

std::vector<entt::entity> query_point(entt::registry& registry, glm::vec2 point, std::uint32_t mask = 0xffffffff);

}
