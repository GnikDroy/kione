#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <imgui.h>

#include "core/entity_editor.hpp"
#include "core/rendering/camera.hpp"

struct Transform {
    glm::vec3 position;
    glm::vec3 scale { 1.0f };
    glm::vec3 rotation;
};

struct FPCamera {
    k2::Camera camera;
    glm::vec3 direction;
    void update() { camera.target = camera.position + direction; }
};

struct DirectionalLight {
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct PointLight {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

namespace k2 {
template <> void ComponentEditorWidget<Transform>(entt::registry& reg, entt::registry::entity_type e) {
    auto& transform = reg.get<Transform>(e);
    ImGui::DragFloat3("Position", glm::value_ptr(transform.position), 0.2f);
    ImGui::DragFloat3("Rotation", glm::value_ptr(transform.rotation), 0.2f);
    ImGui::DragFloat3("Scale", glm::value_ptr(transform.scale), 0.2f);
}

template <> void ComponentEditorWidget<PointLight>(entt::registry& reg, entt::registry::entity_type e) {
    auto& light = reg.get<PointLight>(e);
    ImGui::DragFloat3("Ambient", glm::value_ptr(light.ambient), 0.05f);
    ImGui::DragFloat3("Diffuse", glm::value_ptr(light.diffuse), 0.05f);
    ImGui::DragFloat3("Specular", glm::value_ptr(light.specular), 0.05f);
}

template <> void ComponentEditorWidget<DirectionalLight>(entt::registry& reg, entt::registry::entity_type e) {
    auto& light = reg.get<DirectionalLight>(e);
    ImGui::DragFloat3("Direction", glm::value_ptr(light.direction), 0.05f);
    ImGui::DragFloat3("Ambient", glm::value_ptr(light.ambient), 0.05f);
    ImGui::DragFloat3("Diffuse", glm::value_ptr(light.diffuse), 0.05f);
    ImGui::DragFloat3("Specular", glm::value_ptr(light.specular), 0.05f);
}

}
