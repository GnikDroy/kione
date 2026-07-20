#include "core/collision.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <variant>

#include <glm/gtc/quaternion.hpp>

namespace k2::collision {
namespace {
    struct Circle {
        glm::vec2 center;
        float radius;
    };
    struct Box {
        glm::vec2 center;
        glm::vec2 half;
        float angle;
    };
    struct Segment {
        glm::vec2 start;
        glm::vec2 end;
        float radius;
    };

    glm::vec2 rotate(glm::vec2 vec, float angle) {
        float cos_a = std::cos(angle);
        float sin_a = std::sin(angle);
        return { cos_a * vec.x - sin_a * vec.y, sin_a * vec.x + cos_a * vec.y };
    }

    glm::vec2 closest_point_on_segment(glm::vec2 point, glm::vec2 start, glm::vec2 end) {
        auto direction = end - start;
        float length2 = glm::dot(direction, direction);
        if (length2 <= 0.0f) {
            return start;
        }
        float t = std::clamp(glm::dot(point - start, direction) / length2, 0.0f, 1.0f);
        return start + direction * t;
    }

    // Closest-point-of-two-segments (Ericson, Real-Time Collision Detection 5.1.9).
    float segment_segment_distance2(glm::vec2 p1, glm::vec2 q1, glm::vec2 p2, glm::vec2 q2) {
        constexpr float epsilon = 1e-8f;
        auto d1 = q1 - p1;
        auto d2 = q2 - p2;
        auto r = p1 - p2;
        float a = glm::dot(d1, d1);
        float e = glm::dot(d2, d2);
        float f = glm::dot(d2, r);
        float s = 0.0f;
        float t = 0.0f;
        if (a <= epsilon && e <= epsilon) {
            // Both segments degenerate to points.
        } else if (a <= epsilon) {
            t = std::clamp(f / e, 0.0f, 1.0f);
        } else {
            float c = glm::dot(d1, r);
            if (e <= epsilon) {
                s = std::clamp(-c / a, 0.0f, 1.0f);
            } else {
                float b = glm::dot(d1, d2);
                float denom = a * e - b * b;
                s = denom > epsilon ? std::clamp((b * f - c * e) / denom, 0.0f, 1.0f) : 0.0f;
                t = (b * s + f) / e;
                if (t < 0.0f) {
                    t = 0.0f;
                    s = std::clamp(-c / a, 0.0f, 1.0f);
                } else if (t > 1.0f) {
                    t = 1.0f;
                    s = std::clamp((b - c) / a, 0.0f, 1.0f);
                }
            }
        }
        auto closest1 = p1 + d1 * s;
        auto closest2 = p2 + d2 * t;
        return glm::dot(closest1 - closest2, closest1 - closest2);
    }

    bool segment_intersects_aabb(glm::vec2 start, glm::vec2 end, glm::vec2 half) {
        constexpr float epsilon = 1e-8f;
        auto direction = end - start;
        float t_min = 0.0f;
        float t_max = 1.0f;
        for (int axis = 0; axis < 2; ++axis) {
            if (std::abs(direction[axis]) <= epsilon) {
                if (start[axis] < -half[axis] || start[axis] > half[axis]) {
                    return false;
                }
                continue;
            }
            float t1 = (-half[axis] - start[axis]) / direction[axis];
            float t2 = (half[axis] - start[axis]) / direction[axis];
            if (t1 > t2) {
                std::swap(t1, t2);
            }
            t_min = std::max(t_min, t1);
            t_max = std::min(t_max, t2);
            if (t_min > t_max) {
                return false;
            }
        }
        return true;
    }

    std::array<glm::vec2, 4> box_corners(const Box& box) {
        return { box.center + rotate({ -box.half.x, -box.half.y }, box.angle),
            box.center + rotate({ box.half.x, -box.half.y }, box.angle),
            box.center + rotate({ box.half.x, box.half.y }, box.angle),
            box.center + rotate({ -box.half.x, box.half.y }, box.angle) };
    }

    bool overlap(const Circle& a, const Circle& b) {
        auto delta = a.center - b.center;
        float reach = a.radius + b.radius;
        return glm::dot(delta, delta) <= reach * reach;
    }

    bool overlap(const Circle& circle, const Box& box) {
        auto local = rotate(circle.center - box.center, -box.angle);
        auto delta = local - glm::clamp(local, -box.half, box.half);
        return glm::dot(delta, delta) <= circle.radius * circle.radius;
    }

    bool overlap(const Box& a, const Box& b) {
        std::array axes { rotate({ 1.0f, 0.0f }, a.angle), rotate({ 0.0f, 1.0f }, a.angle),
            rotate({ 1.0f, 0.0f }, b.angle), rotate({ 0.0f, 1.0f }, b.angle) };
        auto corners_a = box_corners(a);
        auto corners_b = box_corners(b);
        for (auto axis : axes) {
            auto project = [&](const std::array<glm::vec2, 4>& corners) {
                float low = std::numeric_limits<float>::max();
                float high = std::numeric_limits<float>::lowest();
                for (auto corner : corners) {
                    float value = glm::dot(corner, axis);
                    low = std::min(low, value);
                    high = std::max(high, value);
                }
                return std::pair { low, high };
            };
            auto [low_a, high_a] = project(corners_a);
            auto [low_b, high_b] = project(corners_b);
            if (high_a < low_b || high_b < low_a) {
                return false;
            }
        }
        return true;
    }

    bool overlap(const Circle& circle, const Segment& pill) {
        auto delta = circle.center - closest_point_on_segment(circle.center, pill.start, pill.end);
        float reach = circle.radius + pill.radius;
        return glm::dot(delta, delta) <= reach * reach;
    }

    bool overlap(const Segment& a, const Segment& b) {
        float reach = a.radius + b.radius;
        return segment_segment_distance2(a.start, a.end, b.start, b.end) <= reach * reach;
    }

    bool overlap(const Box& box, const Segment& pill) {
        auto start = rotate(pill.start - box.center, -box.angle);
        auto end = rotate(pill.end - box.center, -box.angle);
        if (segment_intersects_aabb(start, end, box.half)) {
            return true;
        }
        std::array<glm::vec2, 4> corners { glm::vec2 { -box.half.x, -box.half.y }, { box.half.x, -box.half.y },
            { box.half.x, box.half.y }, { -box.half.x, box.half.y } };
        float best = std::numeric_limits<float>::max();
        for (std::size_t i = 0; i < corners.size(); ++i) {
            best = std::min(best, segment_segment_distance2(start, end, corners[i], corners[(i + 1) % corners.size()]));
        }
        return best <= pill.radius * pill.radius;
    }

    bool overlap(const Box& box, const Circle& circle) { return overlap(circle, box); }
    bool overlap(const Segment& pill, const Circle& circle) { return overlap(circle, pill); }
    bool overlap(const Segment& pill, const Box& box) { return overlap(box, pill); }

    template <class... Ts> struct overloaded : Ts... {
        using Ts::operator()...;
    };

    using Resolved = std::variant<Box, Circle, Segment>;

    Resolved resolve(const WorldCollider& world) {
        return std::visit(
            overloaded {
                [&](const BoxShape& shape) -> Resolved {
                    return Box { .center = world.center, .half = shape.size * 0.5f, .angle = world.angle };
                },
                [&](const CircleShape& shape) -> Resolved {
                    return Circle { .center = world.center, .radius = shape.radius };
                },
                [&](const PillShape& shape) -> Resolved {
                    auto up = rotate({ 0.0f, shape.half_height }, world.angle);
                    return Segment { .start = world.center + up, .end = world.center - up, .radius = shape.radius };
                },
            },
            world.collider.shape);
    }

}

WorldCollider world_collider(const ColliderComponent& collider, const TransformComponent& transform) {
    return { .collider = collider,
        .center = glm::vec2 { transform.translation },
        .angle = glm::eulerAngles(transform.orientation).z };
}

bool overlaps_maskless(const WorldCollider& a, const WorldCollider& b) {
    return std::visit([](const auto& lhs, const auto& rhs) { return overlap(lhs, rhs); }, resolve(a), resolve(b));
}

bool overlaps(entt::registry& registry, entt::entity a, entt::entity b) {
    const auto* collider_a = registry.try_get<ColliderComponent>(a);
    const auto* transform_a = registry.try_get<TransformComponent>(a);
    const auto* collider_b = registry.try_get<ColliderComponent>(b);
    const auto* transform_b = registry.try_get<TransformComponent>(b);
    if (collider_a == nullptr || transform_a == nullptr || collider_b == nullptr || transform_b == nullptr) {
        return false;
    }
    if ((collider_a->mask & collider_b->layer) == 0 || (collider_b->mask & collider_a->layer) == 0) {
        return false;
    }
    return overlaps_maskless(world_collider(*collider_a, *transform_a), world_collider(*collider_b, *transform_b));
}

namespace {

    std::vector<entt::entity> query(entt::registry& registry, const WorldCollider& probe, std::uint32_t mask) {
        std::vector<entt::entity> result;
        for (auto [entity, collider, transform] : registry.view<ColliderComponent, TransformComponent>().each()) {
            if ((collider.layer & mask) != 0 && overlaps_maskless(probe, world_collider(collider, transform))) {
                result.push_back(entity);
            }
        }
        return result;
    }

}

std::vector<entt::entity> query_circle(entt::registry& registry, glm::vec2 center, float radius, std::uint32_t mask) {
    return query(
        registry, { .collider = { .shape = CircleShape { .radius = radius } }, .center = center, .angle = 0.0f }, mask);
}

std::vector<entt::entity> query_aabb(
    entt::registry& registry, glm::vec2 center, glm::vec2 half_extents, std::uint32_t mask) {
    return query(registry,
        { .collider = { .shape = BoxShape { .size = half_extents * 2.0f } }, .center = center, .angle = 0.0f }, mask);
}

std::vector<entt::entity> query_point(entt::registry& registry, glm::vec2 point, std::uint32_t mask) {
    return query_circle(registry, point, 0.0f, mask);
}

}
