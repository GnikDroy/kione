#pragma once

#include <memory>

#include "asset/asset_registry.hpp"

namespace k2 {
class Window;
struct Event;
struct Scene;

class ScriptSystem {
    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    explicit ScriptSystem(Window& window);
    ~ScriptSystem();

    ScriptSystem(const ScriptSystem&) = delete;
    ScriptSystem& operator=(const ScriptSystem&) = delete;

    void update(Scene& scene, const AssetRegistry& assets, float dt);

    void fixed_update(Scene& scene, const AssetRegistry& assets, float dt);

    bool handle_event(Scene& scene, const AssetRegistry& assets, const Event* event);

    void set_input_enabled(bool enabled);
};
}
