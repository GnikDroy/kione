#pragma once

#include <cstdint>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <variant>

#include "core/utils.hpp"

namespace k2 {
struct MainCamera { };

enum class ScaleMode : std::uint8_t { Stretch, FitWidth, FitHeight, Expand, Letterbox };

struct SceneView;

struct Camera {
    enum class Projection : uint8_t { Perspective, Orthographic };

    struct PerspectiveTraits {
        float fov = glm::radians(60.0f);
        float aspect_ratio = 16.0f / 9.0f;
        float far_clip = 1000.f;
        float near_clip = 0.1f;
    };

    struct OrthographicTraits {
        float left = -640.0f;
        float right = 640.0f;
        float top = 360.0f;
        float bottom = -360.0f;
        float far_clip = 0.f;
        float near_clip = 2000.0f;
    };


    glm::vec3 position { 0, 0, 1000.f };
    glm::vec3 target { 0, 0, 0 };
    glm::vec3 up { 0, 1.0f, 0 };

    std::variant<PerspectiveTraits, OrthographicTraits> projection_traits { OrthographicTraits {} };

    ScaleMode scale_mode { ScaleMode::FitHeight };

    [[nodiscard]] glm::mat4 get_view() const { return glm::lookAt(position, target, up); }

    [[nodiscard]] float aspect_ratio() const {
        if (const auto* ortho = std::get_if<OrthographicTraits>(&projection_traits)) {
            float height = ortho->top - ortho->bottom;
            return height != 0.0f ? (ortho->right - ortho->left) / height : 1.0f;
        }
        return std::get<PerspectiveTraits>(projection_traits).aspect_ratio;
    }

    [[nodiscard]] glm::mat4 get_projection() const {
        return std::visit(
            [](auto&& traits) {
                using T = std::decay_t<decltype(traits)>;
                if constexpr (std::is_same_v<T, PerspectiveTraits>) {
                    return glm::perspective(traits.fov, traits.aspect_ratio, traits.near_clip, traits.far_clip);
                } else if constexpr (std::is_same_v<T, OrthographicTraits>) {
                    return glm::ortho(
                        traits.left, traits.right, traits.bottom, traits.top, traits.near_clip, traits.far_clip);
                } else {
                    static_assert(always_false<T>, "Invalid variant!");
                }
            },
            projection_traits);
    }

    [[nodiscard]] glm::mat4 get_view_projection() const { return get_projection() * get_view(); }

    [[nodiscard]] SceneView for_surface(float surface_w, float surface_h) const;
};

// The view a scene is currently rendered with, published into the registry ctx each frame.
// Screen<->world conversion lives here because it needs both the camera and its viewport rect.
struct SceneView {
    Camera camera {};
    Rect<float> viewport {};

    [[nodiscard]] glm::vec2 screen_to_world(glm::vec2 screen) const {
        glm::vec2 ndc { (screen.x - viewport.x) / viewport.w * 2.0f - 1.0f,
            1.0f - (screen.y - viewport.y) / viewport.h * 2.0f };
        auto world = glm::inverse(camera.get_view_projection()) * glm::vec4 { ndc, 0.0f, 1.0f };
        return glm::vec2 { world } / world.w;
    }

    [[nodiscard]] glm::vec2 world_to_screen(glm::vec2 world) const {
        auto clip = camera.get_view_projection() * glm::vec4 { world, 0.0f, 1.0f };
        auto ndc = glm::vec2 { clip } / clip.w;
        return { viewport.x + (ndc.x + 1.0f) * 0.5f * viewport.w, viewport.y + (1.0f - ndc.y) * 0.5f * viewport.h };
    }
};

inline Rect<float> letterbox_fit(float outer_w, float outer_h, float design_aspect) {
    if (outer_h <= 0.0f || design_aspect <= 0.0f) {
        return { .x = 0.0f, .y = 0.0f, .w = outer_w, .h = outer_h };
    }
    if (outer_w / outer_h > design_aspect) {
        float width = outer_h * design_aspect;
        return { .x = (outer_w - width) * 0.5f, .y = 0.0f, .w = width, .h = outer_h };
    }
    float height = outer_w / design_aspect;
    return { .x = 0.0f, .y = (outer_h - height) * 0.5f, .w = outer_w, .h = height };
}

inline SceneView Camera::for_surface(float surface_w, float surface_h) const {
    SceneView resolved { .camera = *this, .viewport = { .x = 0.0f, .y = 0.0f, .w = surface_w, .h = surface_h } };
    if (surface_w <= 0.0f || surface_h <= 0.0f || scale_mode == ScaleMode::Stretch) {
        return resolved;
    }
    float surface_aspect = surface_w / surface_h;

    if (auto* ortho = std::get_if<OrthographicTraits>(&resolved.camera.projection_traits)) {
        float half_w = (ortho->right - ortho->left) * 0.5f;
        float half_h = (ortho->top - ortho->bottom) * 0.5f;
        float center_x = (ortho->right + ortho->left) * 0.5f;
        float center_y = (ortho->top + ortho->bottom) * 0.5f;
        float aspect = half_h != 0.0f ? half_w / half_h : 1.0f;
        switch (scale_mode) {
        case ScaleMode::FitHeight: half_w = half_h * surface_aspect; break;
        case ScaleMode::FitWidth: half_h = half_w / surface_aspect; break;
        case ScaleMode::Expand:
            if (surface_aspect > aspect) {
                half_w = half_h * surface_aspect;
            } else {
                half_h = half_w / surface_aspect;
            }
            break;
        case ScaleMode::Letterbox:
            resolved.viewport = letterbox_fit(surface_w, surface_h, aspect);
            return resolved;
       case ScaleMode::Stretch: return resolved;
        }
        ortho->left = center_x - half_w;
        ortho->right = center_x + half_w;
        ortho->bottom = center_y - half_h;
        ortho->top = center_y + half_h;
        return resolved;
    }

    auto& perspective = std::get<PerspectiveTraits>(resolved.camera.projection_traits);
    if (scale_mode == ScaleMode::Letterbox) {
        resolved.viewport = letterbox_fit(surface_w, surface_h, perspective.aspect_ratio);
    } else {
        perspective.aspect_ratio = surface_aspect;
    }
    return resolved;
}

template <class EntityType> const Camera* find_main_camera(const entt::basic_registry<EntityType>& registry) {
    for (auto entity : registry.template view<Camera, MainCamera>()) {
        return &registry.template get<Camera>(entity);
    }
    return nullptr;
}
}
