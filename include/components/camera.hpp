#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <variant>

#include "core/utils.hpp"

namespace k2 {
struct Camera {
    enum class Projection { Perspective, Orthographic };

    template <auto T> struct ProjectionTraits;

    template <> struct ProjectionTraits<Projection::Perspective> {
        float fov;
        float aspect_ratio;
        float far_clip;
        float near_clip;
    };

    template <> struct ProjectionTraits<Projection::Orthographic> {
        float left;
        float right;
        float top;
        float bottom;
        float far_clip = -1000.0f;
        float near_clip = 1000.0f;
    };

    using PerspectiveTraits = ProjectionTraits<Projection::Perspective>;
    using OrthographicTraits = ProjectionTraits<Projection::Orthographic>;

    glm::vec3 position { 0, 0, 10.f };
    glm::vec3 target { 0, 0, 0 };
    glm::vec3 up { 0, 1.0f, 0 };

    std::variant<PerspectiveTraits, OrthographicTraits> projection_traits;

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
}
