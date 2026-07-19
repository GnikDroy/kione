#pragma once
#include <string>

#include <entt/entt.hpp>

namespace k2 {
struct SceneRequest {
    std::string scene;
};

struct Scene {
    entt::registry registry;

    Scene() = default;
    Scene(Scene&& scene) noexcept
        : registry { std::move(scene.registry) } { }
    Scene& operator=(Scene&& scene) noexcept {
        std::swap(registry, scene.registry);
        return *this;
    }
};
}
