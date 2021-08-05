#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <imgui.h>

#include "components/camera.hpp"

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