#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <variant>

#include "core/utils.hpp"

namespace k2 {
struct MainCamera { };

struct Camera {
    enum class Projection : uint8_t { Perspective, Orthographic };

    template <auto T> struct ProjectionTraits;

    template <> struct ProjectionTraits<Projection::Perspective> {
        float fov = glm::radians(60.0f);
        float aspect_ratio = 16.0f / 9.0f;
        float far_clip = 1000.f;
        float near_clip = 0.1f;
    };

    template <> struct ProjectionTraits<Projection::Orthographic> {
        float left = -640.0f;
        float right = 640.0f;
        float top = 360.0f;
        float bottom = -360.0f;
        float far_clip = 0.f;
        float near_clip = 2000.0f;
    };

    using PerspectiveTraits = ProjectionTraits<Projection::Perspective>;
    using OrthographicTraits = ProjectionTraits<Projection::Orthographic>;

    glm::vec3 position { 0, 0, 1000.f };
    glm::vec3 target { 0, 0, 0 };
    glm::vec3 up { 0, 1.0f, 0 };

    std::variant<PerspectiveTraits, OrthographicTraits> projection_traits { OrthographicTraits {} };

    glm::mat4 get_view() { return glm::lookAt(position, target, up); }

    glm::mat4 get_projection() {
        return std::visit(
            [](auto&& traits) {
                using T = std::decay_t<decltype(traits)>;
                if constexpr (std::is_same_v<T, ProjectionTraits<Projection::Perspective>>) {
                    return glm::perspective(traits.fov, traits.aspect_ratio, traits.near_clip, traits.far_clip);
                } else if constexpr (std::is_same_v<T, ProjectionTraits<Projection::Orthographic>>) {
                    return glm::ortho(
                        traits.left, traits.right, traits.bottom, traits.top, traits.near_clip, traits.far_clip);
                } else {
                    static_assert(always_false<T>, "Invalid variant!");
                }
            },
            projection_traits);
    }

    glm::mat4 get_view_projection() { return get_projection() * get_view(); }
};

template <class EntityType>
const Camera* find_main_camera(const entt::basic_registry<EntityType>& registry) {
    for (auto entity : registry.template view<Camera, MainCamera>()) {
        return &registry.template get<Camera>(entity);
    }
    return nullptr;
}
}
