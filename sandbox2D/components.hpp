#pragma once
#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>


#include "entt/entt.hpp"
#include "imgui.h"
#include "core/entity_editor.hpp"

struct Transform {
    glm::vec3 position;
    glm::vec2 rotation;
    glm::vec2 scale;
};

namespace k2 {
    template<>
    void ComponentEditorWidget<Transform>(entt::registry &reg, entt::registry::entity_type e) {
        auto &transform = reg.get<Transform>(e);
        ImGui::DragFloat3("Position", glm::value_ptr(transform.position), 0.2f);
        ImGui::DragFloat2("Rotation", glm::value_ptr(transform.rotation), 0.2f);
        ImGui::DragFloat2("Scale", glm::value_ptr(transform.scale), 0.2f);
    }
}

