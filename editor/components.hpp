#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

struct Transform {
    glm::vec3 position;
    glm::vec3 scale { 1.0f };
    glm::vec3 rotation;
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
template <> void ComponentInspectorWidget<Transform>(entt::registry& reg, entt::registry::entity_type e) {
    auto& transform = reg.get<Transform>(e);
    ImGui::DragFloat3("Position", glm::value_ptr(transform.position), 0.2f);
    ImGui::DragFloat3("Rotation", glm::value_ptr(transform.rotation), 0.2f);
    ImGui::DragFloat3("Scale", glm::value_ptr(transform.scale), 0.2f);
}

template <> void ComponentInspectorWidget<PointLight>(entt::registry& reg, entt::registry::entity_type e) {
    auto& light = reg.get<PointLight>(e);
    ImGui::DragFloat3("Ambient", glm::value_ptr(light.ambient), 0.05f);
    ImGui::DragFloat3("Diffuse", glm::value_ptr(light.diffuse), 0.05f);
    ImGui::DragFloat3("Specular", glm::value_ptr(light.specular), 0.05f);
}

template <> void ComponentInspectorWidget<DirectionalLight>(entt::registry& reg, entt::registry::entity_type e) {
    auto& light = reg.get<DirectionalLight>(e);
    ImGui::DragFloat3("Direction", glm::value_ptr(light.direction), 0.05f);
    ImGui::DragFloat3("Ambient", glm::value_ptr(light.ambient), 0.05f);
    ImGui::DragFloat3("Diffuse", glm::value_ptr(light.diffuse), 0.05f);
    ImGui::DragFloat3("Specular", glm::value_ptr(light.specular), 0.05f);
}

}
