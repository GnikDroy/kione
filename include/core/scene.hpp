#pragma once
#include <entt/entt.hpp>

#include "asset/asset.hpp"

namespace k2 {
struct Scene {
    entt::registry registry;

    Scene() = default;
    Scene(Scene&& scene)
        : registry { std::move(scene.registry) } { }
    Scene& operator=(Scene&& scene) {
        std::swap(registry, scene.registry);
        return *this;
    }
};
}