#pragma once
#include <entt/entt.hpp>
#include <imgui.h>

#include "components/animation.hpp"
#include "components/camera.hpp"
#include "components/light.hpp"
#include "components/script.hpp"
#include "components/sprite.hpp"
#include "components/tag.hpp"
#include "components/transform.hpp"

#include "ui/common.hpp"
#include "ui/widgets/component_inspector.hpp"

namespace k2::editor {
template <> void ComponentWidget<k2::TransformComponent>(entt::registry& reg, entt::registry::entity_type e) {
    auto& transform = reg.get<TransformComponent>(e);
    if (BeginPropertyTable("Transform")) {
        PropertyLabel("Translation");
        Vec3Field("##Translation", transform.translation);
        PropertyLabel("Rotation");
        RotationField("##Rotation", transform.orientation);
        PropertyLabel("Scale");
        Vec3Field("##Scale", transform.scale, { 1.0f, 1.0f, 1.0f });
        EndPropertyTable();
    }
}

template <> void ComponentWidget<k2::Camera>(entt::registry& reg, entt::registry::entity_type e) {
    auto& camera = reg.get<k2::Camera>(e);
    if (!BeginPropertyTable("Camera")) {
        return;
    }
    PropertyLabel("Position");
    Vec3Field("##Position", camera.position);
    PropertyLabel("Target");
    Vec3Field("##Target", camera.target);
    PropertyLabel("Up");
    Vec3Field("##Up", camera.up, { 0.0f, 1.0f, 0.0f });

    constexpr auto items = []() constexpr {
        std::array<const char*, 2> items {};
        items[int(k2::Camera::Projection::Perspective)] = "Perspective";
        items[int(k2::Camera::Projection::Orthographic)] = "Orthographic";
        return items;
    }();
    int item = int(camera.projection_traits.index());
    PropertyLabel("Projection");
    ImGui::Combo("##Projection", &item, items.data(), int(items.size()));

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
                PropertyLabel("FOV");
                ImGui::DragFloat("##FOV", &traits.fov, 0.01f);
                PropertyLabel("Aspect Ratio");
                ImGui::DragFloat("##AspectRatio", &traits.aspect_ratio, 0.01f);
                PropertyLabel("Near Clip");
                ImGui::DragFloat("##NearClip", &traits.near_clip, 1.0f);
                PropertyLabel("Far Clip");
                ImGui::DragFloat("##FarClip", &traits.far_clip, 1.0f);
            } else if constexpr (std::is_same_v<T,
                                     k2::Camera::ProjectionTraits<k2::Camera::Projection::Orthographic>>) {
                PropertyLabel("Left");
                ImGui::DragFloat("##Left", &traits.left, 1.0f);
                PropertyLabel("Right");
                ImGui::DragFloat("##Right", &traits.right, 1.0f);
                PropertyLabel("Bottom");
                ImGui::DragFloat("##Bottom", &traits.bottom, 1.0f);
                PropertyLabel("Top");
                ImGui::DragFloat("##Top", &traits.top, 1.0f);
                PropertyLabel("Near Clip");
                ImGui::DragFloat("##NearClip", &traits.near_clip, 1.0f);
                PropertyLabel("Far Clip");
                ImGui::DragFloat("##FarClip", &traits.far_clip, 1.0f);
            } else {
                static_assert(always_false<T>, "Invalid variant!");
            }
        },
        camera.projection_traits);
    EndPropertyTable();
}

template <> void ComponentWidget<k2::SpriteComponent>(entt::registry& reg, entt::registry::entity_type e) {
    auto& sprite = reg.get<k2::SpriteComponent>(e);
    auto& editor_layer = reg.ctx().get<EditorLayer&>();
    if (BeginPropertyTable("Sprite")) {
        PropertyLabel("Color");
        ImGui::ColorEdit4("##Color", glm::value_ptr(sprite.color));
        PropertyLabel("Texture");
        ResourceInputWidget("##Texture", sprite.texture, editor_layer.active_assets(), k2::Asset::Type::Image);
        PropertyLabel("UV Rect");
        RectField("##UvRect", sprite.uv_rect, { 0.0f, 0.0f, 1.0f, 1.0f });
        EndPropertyTable();
    }
}

template <> void ComponentWidget<k2::AnimationComponent>(entt::registry& reg, entt::registry::entity_type e) {
    auto& animation = reg.get<k2::AnimationComponent>(e);
    auto& editor_layer = reg.ctx().get<EditorLayer&>();
    if (BeginPropertyTable("Animation")) {
        PropertyLabel("Clip");
        ResourceInputWidget("##Clip", animation.clip, editor_layer.active_assets(), k2::Asset::Type::Animation);
        PropertyLabel("Speed");
        ImGui::DragFloat("##Speed", &animation.speed, 0.01f, -100.0f, 100.0f);
        PropertyLabel("Playing");
        ImGui::Checkbox("##Playing", &animation.playing);
        EndPropertyTable();
    }
}

template <> void ComponentWidget<k2::MainCamera>(entt::registry&, entt::registry::entity_type) {
    ImGui::TextDisabled("Marks the camera the renderer uses.");
}

template <> void ComponentWidget<k2::ScriptComponent>(entt::registry& reg, entt::registry::entity_type e) {
    auto& script = reg.get<k2::ScriptComponent>(e);
    auto& editor_layer = reg.ctx().get<EditorLayer&>();
    if (BeginPropertyTable("Script")) {
        PropertyLabel("Script");
        ResourceInputWidget("##Script", script.script, editor_layer.active_assets(), k2::Asset::Type::Script);
        EndPropertyTable();
    }
}

template <> void ComponentWidget<k2::TagComponent>(entt::registry& reg, entt::registry::entity_type e) {
    auto& tag = reg.get<k2::TagComponent>(e).tag;
    if (BeginPropertyTable("Tag")) {
        PropertyLabel("Tag");
        ImGui::InputText("##Tag", &tag);
        EndPropertyTable();
    }
}

template <> void ComponentWidget<k2::AmbientLight>(entt::registry& reg, entt::registry::entity_type e) {
    auto& light = reg.get<k2::AmbientLight>(e);
    if (BeginPropertyTable("AmbientLight")) {
        PropertyLabel("Color");
        ImGui::ColorEdit3("##Color", glm::value_ptr(light.color));
        PropertyLabel("Intensity");
        ImGui::DragFloat("##Intensity", &light.intensity, 0.01f, 0.0f, 100.0f);
        EndPropertyTable();
    }
}

template <> void ComponentWidget<k2::PointLight>(entt::registry& reg, entt::registry::entity_type e) {
    auto& light = reg.get<k2::PointLight>(e);
    if (BeginPropertyTable("PointLight")) {
        PropertyLabel("Color");
        ImGui::ColorEdit3("##Color", glm::value_ptr(light.color));
        PropertyLabel("Intensity");
        ImGui::DragFloat("##Intensity", &light.intensity, 0.01f, 0.0f, 100.0f);
        PropertyLabel("Radius");
        ImGui::DragFloat("##Radius", &light.radius, 1.0f, 0.0f, 100000.0f);
        EndPropertyTable();
    }
}

template <> void ComponentWidget<k2::SpotLight>(entt::registry& reg, entt::registry::entity_type e) {
    auto& light = reg.get<k2::SpotLight>(e);
    if (BeginPropertyTable("SpotLight")) {
        PropertyLabel("Color");
        ImGui::ColorEdit3("##Color", glm::value_ptr(light.color));
        PropertyLabel("Intensity");
        ImGui::DragFloat("##Intensity", &light.intensity, 0.01f, 0.0f, 100.0f);
        PropertyLabel("Radius");
        ImGui::DragFloat("##Radius", &light.radius, 1.0f, 0.0f, 100000.0f);
        PropertyLabel("Inner Angle");
        if (ImGui::SliderAngle("##InnerAngle", &light.inner_angle, 0.0f, 180.0f)) {
            light.outer_angle = std::max(light.outer_angle, light.inner_angle);
        }
        PropertyLabel("Outer Angle");
        if (ImGui::SliderAngle("##OuterAngle", &light.outer_angle, 0.0f, 180.0f)) {
            light.inner_angle = std::min(light.inner_angle, light.outer_angle);
        }
        EndPropertyTable();
    }
}

template <> void ComponentWidget<k2::SpriteLight>(entt::registry& reg, entt::registry::entity_type e) {
    auto& light = reg.get<k2::SpriteLight>(e);
    auto& editor_layer = reg.ctx().get<EditorLayer&>();
    if (BeginPropertyTable("SpriteLight")) {
        PropertyLabel("Texture");
        ResourceInputWidget("##Texture", light.texture, editor_layer.active_assets(), k2::Asset::Type::Image);
        PropertyLabel("Color");
        ImGui::ColorEdit3("##Color", glm::value_ptr(light.color));
        PropertyLabel("Intensity");
        ImGui::DragFloat("##Intensity", &light.intensity, 0.01f, 0.0f, 100.0f);
        EndPropertyTable();
    }
}
}
