#pragma once
#include <entt/entt.hpp>

namespace k2 {
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
