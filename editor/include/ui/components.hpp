#pragma once
#include <entt/entt.hpp>
#include <imgui.h>

#include "components/camera.hpp"
#include "components/light.hpp"
#include "components/relation.hpp"
#include "components/script.hpp"
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
    ResourceInputWidget("Texture", sprite.texture, editor_layer.active_assets(), k2::Asset::Type::Image);

    k2::editor::RectInputWidget("UV Rect", sprite.uv_rect);
}

template <> void ComponentWidget<k2::MainCamera>(entt::registry&, entt::registry::entity_type) { }

template <> void ComponentWidget<k2::ScriptComponent>(entt::registry& reg, entt::registry::entity_type e) {
    auto& script = reg.get<k2::ScriptComponent>(e);
    auto& editor_layer = reg.ctx().get<EditorLayer&>();
    ResourceInputWidget("Script", script.script, editor_layer.active_assets(), k2::Asset::Type::Script);
}

template <> void ComponentWidget<k2::TagComponent>(entt::registry& reg, entt::registry::entity_type e) {
    auto& tag = reg.get<k2::TagComponent>(e).tag;
    ImGui::InputText("Tag", &tag);
}

template <> void ComponentWidget<k2::AmbientLight>(entt::registry& reg, entt::registry::entity_type e) {
    auto& light = reg.get<k2::AmbientLight>(e);
    ImGui::ColorEdit3("Color", glm::value_ptr(light.color));
    ImGui::DragFloat("Intensity", &light.intensity, 0.01f, 0.0f, 100.0f);
}

template <> void ComponentWidget<k2::PointLight>(entt::registry& reg, entt::registry::entity_type e) {
    auto& light = reg.get<k2::PointLight>(e);
    ImGui::ColorEdit3("Color", glm::value_ptr(light.color));
    ImGui::DragFloat("Intensity", &light.intensity, 0.01f, 0.0f, 100.0f);
    ImGui::DragFloat("Radius", &light.radius, 1.0f, 0.0f, 100000.0f);
}

template <> void ComponentWidget<k2::SpotLight>(entt::registry& reg, entt::registry::entity_type e) {
    auto& light = reg.get<k2::SpotLight>(e);
    ImGui::ColorEdit3("Color", glm::value_ptr(light.color));
    ImGui::DragFloat("Intensity", &light.intensity, 0.01f, 0.0f, 100.0f);
    ImGui::DragFloat("Radius", &light.radius, 1.0f, 0.0f, 100000.0f);
    ImGui::SliderAngle("Inner Angle", &light.inner_angle, 0.0f, 180.0f);
    ImGui::SliderAngle("Outer Angle", &light.outer_angle, 0.0f, 180.0f);
}

template <> void ComponentWidget<k2::SpriteLight>(entt::registry& reg, entt::registry::entity_type e) {
    auto& light = reg.get<k2::SpriteLight>(e);
    auto& editor_layer = reg.ctx().get<EditorLayer&>();
    ResourceInputWidget("Texture", light.texture, editor_layer.active_assets(), k2::Asset::Type::Image);
    ImGui::ColorEdit3("Color", glm::value_ptr(light.color));
    ImGui::DragFloat("Intensity", &light.intensity, 0.01f, 0.0f, 100.0f);
}
}
