#pragma once

#include <memory>

#include "asset/asset_registry.hpp"

namespace k2 {
class Window;
struct Event;
struct Scene;

// Runs entity Lua scripts. A script file is executed once per entity into its own
// environment, so script-local state is per-entity. Recognized hooks:
// on_create(entity), on_update(entity, dt), on_fixed_update(entity, dt),
// on_destroy(entity),
// on_event(entity, event) — event.type discriminates ("key", "char", "mouse_button",
// "mouse_drop", "cursor_position", "cursor_enter", "scroll", "window_close",
// "window_resize", "framebuffer_resize", "content_scale", "window_reposition",
// "window_iconify", "window_maximize", "window_focus") with the source event's fields;
// return true to consume. Input events are gated on set_input_enabled; window events
// are always delivered.
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

    void clear_cache();
};
}
