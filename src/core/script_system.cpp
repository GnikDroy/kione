#include "core/script_system.hpp"

#include <format>
#include <string>
#include <unordered_map>

#include <sol/sol.hpp>

#include "asset/scheme.hpp"
#include "components/script.hpp"
#include "core/logger.hpp"
#include "core/scene.hpp"
#include "core/script/bindings.hpp"
#include "core/script/event_translation.hpp"
#include "core/script/lua_entity.hpp"

namespace k2 {

struct ScriptSystem::Impl {
    sol::state lua;
    std::unordered_map<entt::entity, sol::environment> instances;
    std::unordered_map<ResourceID, std::string> sources;
    entt::registry* attached {};
    bool input_enabled { true };

    explicit Impl(Window& window) {
        lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table);
        bind_script_api(lua, window, input_enabled);
    }

    const std::string& source_for(const AssetHandle& handle, const AssetRegistry& assets) {
        auto it = sources.find(handle.id);
        if (it != sources.end()) {
            return it->second;
        }

        std::string source;
        auto asset_it = assets.find(handle.id);
        if (asset_it == assets.end() || asset_it->second.second.type != Asset::Type::Script) {
            Log::core().warn(std::format("Scene references unknown script asset '{}'", handle.name));
        } else {
            try {
                auto raw = AssetScheme::get_raw(asset_it->second.second);
                source.assign(raw.begin(), raw.end());
            } catch (const std::exception& e) {
                Log::core().error(std::format("Failed to load script '{}': {}", handle.name, e.what()));
            }
        }
        return sources.emplace(handle.id, std::move(source)).first->second;
    }

    template <class... Args>
    bool call_hook(sol::environment& env, const char* hook, const AssetHandle& script, Args&&... args) {
        sol::protected_function function = env[hook];
        if (!function.valid()) {
            return false;
        }
        auto result = function(std::forward<Args>(args)...);
        if (!result.valid()) {
            sol::error err = result;
            Log::core().error(std::format("Script '{}' {}: {}", script.name, hook, err.what()));
            env[hook] = sol::lua_nil;
            return false;
        }
        if (result.return_count() == 0) {
            return false;
        }
        return result.template get<sol::optional<bool>>().value_or(false);
    }

    sol::environment& instance(
        entt::registry& registry, entt::entity entity, const ScriptComponent& script, const AssetRegistry& assets) {
        if (auto it = instances.find(entity); it != instances.end()) {
            return it->second;
        }

        sol::environment env(lua, sol::create, lua.globals());
        const auto& source = source_for(script.script, assets);
        if (!source.empty()) {
            auto result = lua.safe_script(source, env, sol::script_pass_on_error, script.script.name);
            if (!result.valid()) {
                sol::error err = result;
                Log::core().error(std::format("Script '{}' failed to load: {}", script.script.name, err.what()));
            }
        }

        auto& stored = instances.emplace(entity, std::move(env)).first->second;
        call_hook(stored, "on_create", script.script, LuaEntity { .entity = entity, .registry = &registry });
        return stored;
    }

    void ensure_attached(entt::registry& registry) {
        if (attached != &registry) {
            instances.clear();
            registry.on_destroy<ScriptComponent>().connect<&Impl::on_script_destroyed>(*this);
            attached = &registry;
        }
    }

    void on_script_destroyed(entt::registry& registry, entt::entity entity) {
        if (auto it = instances.find(entity); it != instances.end()) {
            auto& script = registry.get<ScriptComponent>(entity);
            call_hook(it->second, "on_destroy", script.script, LuaEntity { .entity = entity, .registry = &registry });
            instances.erase(it);
        }
    }
};

ScriptSystem::ScriptSystem(Window& window)
    : impl { std::make_unique<Impl>(window) } { }

ScriptSystem::~ScriptSystem() = default;

void ScriptSystem::set_input_enabled(bool enabled) { impl->input_enabled = enabled; }

void ScriptSystem::clear_cache() {
    impl->sources.clear();
    impl->instances.clear();
}

void ScriptSystem::update(Scene& scene, const AssetRegistry& assets, float dt) {
    auto& registry = scene.registry;
    impl->ensure_attached(registry);

    for (auto [entity, script] : registry.view<ScriptComponent>().each()) {
        auto& env = impl->instance(registry, entity, script, assets);
        impl->call_hook(env, "on_update", script.script, LuaEntity { .entity = entity, .registry = &registry }, dt);
    }
}

bool ScriptSystem::handle_event(Scene& scene, const AssetRegistry& assets, const Event* event) {
    auto translated = translate_event(impl->lua, event);
    if (!translated || (translated->input && !impl->input_enabled)) {
        return false;
    }
    auto& registry = scene.registry;
    impl->ensure_attached(registry);

    bool consumed = false;
    for (auto [entity, script] : registry.view<ScriptComponent>().each()) {
        auto& env = impl->instance(registry, entity, script, assets);
        consumed |= impl->call_hook(
            env, "on_event", script.script, LuaEntity { .entity = entity, .registry = &registry }, translated->table);
    }
    return consumed;
}

}
