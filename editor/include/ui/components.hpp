#pragma once
#include <entt/entt.hpp>
#include <imgui.h>

#include "components/camera.hpp"
#include "components/relation.hpp"
#include "components/sprite.hpp"
#include "components/tag.hpp"
#include "components/transform.hpp"

#include "ui/common.hpp"
#include "ui/widgets/component_inspector.hpp"

namespace k2::editor {
template <> void ComponentWidget<k2::TransformComponent>(entt::registry& reg, entt::registry::entity_type e) {
    auto& transform = reg.get<TransformComponent>(e);
    k2::editor::Vec3InputWidget("Translation", transform.translation);
    k2::editor::Vec3InputWidget("Scale", transform.scale);
    k2::editor::OrientationInputWidget("Orientation", transform.orientation);
}

template <> void ComponentWidget<k2::Camera>(entt::registry& reg, entt::registry::entity_type e) {
    auto& camera = reg.get<k2::Camera>(e);
    k2::editor::Vec3InputWidget("Position", camera.position);
    k2::editor::Vec3InputWidget("Target", camera.target);
    k2::editor::Vec3InputWidget("Up", camera.up);

    constexpr auto items = []() constexpr {
        std::array<const char*, 2> items {};
        items[int(k2::Camera::Projection::Perspective)] = "Perspective";
        items[int(k2::Camera::Projection::Orthographic)] = "Orthographic";
        return items;
    }();
    int item = int(camera.projection_traits.index());
    ImGui::Combo("Projection Type", &item, items.data(), int(items.size()));

    // Switch to the proper variant according to the combo item if different.
    auto projection_type = k2::Camera::Projection(item);
    switch (projection_type) {
    case k2::Camera::Projection::Perspective: {
        if (!std::holds_alternative<k2::Camera::ProjectionTraits<k2::Camera::Projection::Perspective>>(
                camera.projection_traits)) {
            camera.projection_traits = k2::Camera::ProjectionTraits<k2::Camera::Projection::Perspective> {};
        }
        break;
    }
    case k2::Camera::Projection::Orthographic: {
        if (!std::holds_alternative<k2::Camera::ProjectionTraits<k2::Camera::Projection::Orthographic>>(
                camera.projection_traits)) {
            camera.projection_traits = k2::Camera::ProjectionTraits<k2::Camera::Projection::Orthographic> {};
        }
        break;
    }
    default: assert(false && "Invalid projection type.");
    }

    std::visit(
        [](auto&& traits) {
            using T = std::decay_t<decltype(traits)>;
            if constexpr (std::is_same_v<T, k2::Camera::ProjectionTraits<k2::Camera::Projection::Perspective>>) {
                ImGui::InputFloat("FOV", &traits.fov);
                ImGui::InputFloat("Aspect Ratio", &traits.aspect_ratio);
                ImGui::InputFloat("Near Clip", &traits.near_clip);
                ImGui::InputFloat("Far Clip", &traits.far_clip);

            } else if constexpr (std::is_same_v<T,
                                     k2::Camera::ProjectionTraits<k2::Camera::Projection::Orthographic>>) {
                ImGui::InputFloat("Left", &traits.left);
                ImGui::InputFloat("Right", &traits.right);
                ImGui::InputFloat("Bottom", &traits.bottom);
                ImGui::InputFloat("Top", &traits.top);
                ImGui::InputFloat("Near Clip", &traits.near_clip);
                ImGui::InputFloat("Far Clip", &traits.far_clip);
            } else {
                static_assert(always_false<T>, "Invalid variant!");
            }
        },
        camera.projection_traits);
}

template <> void ComponentWidget<k2::SpriteComponent>(entt::registry& reg, entt::registry::entity_type e) {
    auto& sprite = reg.get<k2::SpriteComponent>(e);
    ImGui::ColorEdit4("Color", glm::value_ptr(sprite.color));

    auto& editor_layer = reg.ctx().get<EditorLayer&>();
    ResourceInputWidget("Texture", sprite.texture, editor_layer.assets);

    k2::editor::RectInputWidget("UV Rect", sprite.uv_rect);
}

template <> void ComponentWidget<k2::MainCamera>(entt::registry&, entt::registry::entity_type) { }

template <> void ComponentWidget<k2::TagComponent>(entt::registry& reg, entt::registry::entity_type e) {
    auto& tag = reg.get<k2::TagComponent>(e).tag;
    ImGui::InputText("Tag", &tag);
}
}
