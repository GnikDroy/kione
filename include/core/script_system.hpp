#pragma once

#include <memory>

#include "asset/asset_registry.hpp"

namespace k2 {
class Window;
struct Scene;

// Runs entity Lua scripts. A script file is executed once per entity into its own
// environment, so script-local state is per-entity. Recognized hooks:
// on_create(entity), on_update(entity, dt), on_destroy(entity).
class ScriptSystem {
    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    explicit ScriptSystem(Window& window);
    ~ScriptSystem();

    ScriptSystem(const ScriptSystem&) = delete;
    ScriptSystem& operator=(const ScriptSystem&) = delete;

    void update(Scene& scene, const AssetRegistry& assets, float dt);
};
}
