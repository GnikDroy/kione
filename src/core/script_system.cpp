#include "core/script_system.hpp"

#include <cstdint>
#include <format>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <sol/sol.hpp>

#include "asset/scheme.hpp"
#include "components/camera.hpp"
#include "components/relation.hpp"
#include "components/script.hpp"
#include "components/transform.hpp"
#include "core/audio.hpp"
#include "core/collision.hpp"
#include "core/entity_ops.hpp"
#include "core/logger.hpp"
#include "core/resources.hpp"
#include "core/scene.hpp"
#include "core/script/bindings.hpp"
#include "core/script/event_translation.hpp"
#include "core/script/host.hpp"
#include "core/script/lua_component.hpp"
#include "core/script/lua_entity.hpp"

namespace k2 {

struct ScriptSystem::Impl : ScriptHost {
    sol::state lua;
    std::unordered_map<entt::entity, sol::environment> instances;
    std::unordered_map<ResourceID, std::string> sources;
    entt::registry* attached {};
    bool input_enabled { true };

    entt::registry* current_registry {};
    const AssetRegistry* current_assets {};

    std::shared_ptr<std::uint64_t> epoch = std::make_shared<std::uint64_t>(0);

    enum class CommandKind : std::uint8_t { AttachScript, Destroy };
    struct Command {
        CommandKind kind;
        entt::entity entity;
        AssetHandle script; // AttachScript only
    };
    std::vector<Command> pending;

    static constexpr std::size_t command_cap = 10000;

    std::unordered_set<std::string> baseline_globals;
    std::unordered_set<std::string> baseline_k2;

    explicit Impl(Window& window) {
        lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table, sol::lib::io,
            sol::lib::os, sol::lib::coroutine);
        bind_script_api(lua, window, input_enabled, *this);
        baseline_globals = table_string_keys(lua.globals());
        baseline_k2 = table_string_keys(lua["k2"]);
    }

    void reset_globals() {
        reset_table_to_baseline(lua.globals(), baseline_globals);
        reset_table_to_baseline(lua["k2"], baseline_k2); // e.g. k2.game
    }

    LuaEntity make_handle(entt::entity entity) {
        return LuaEntity { .entity = entity, .registry = current_registry, .epoch_token = epoch, .stamp = *epoch };
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

    sol::environment& instance(entt::entity entity, const ScriptComponent& script, const AssetRegistry& assets) {
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
        call_hook(stored, "on_create", script.script, make_handle(entity));
        return stored;
    }

    void ensure_attached(entt::registry& registry) {
        if (attached != &registry) {
            instances.clear();
            pending.clear();
            ++*epoch; // invalidate handles held from a prior scene
            registry.on_destroy<ScriptComponent>().connect<&Impl::on_script_destroyed>(*this);
            attached = &registry;
        }
    }

    void on_script_destroyed(entt::registry& registry, entt::entity entity) {
        if (auto it = instances.find(entity); it != instances.end()) {
            auto& script = registry.get<ScriptComponent>(entity);
            call_hook(it->second, "on_destroy", script.script, make_handle(entity));
            instances.erase(it);
        }
    }

    // ScriptComponent pool is not mutated mid-iteration.
    void flush_commands() {
        std::size_t processed = 0;
        while (processed < pending.size()) {
            if (processed >= command_cap) {
                Log::core().error("Script command queue exceeded cap; dropping remaining spawn/destroy requests");
                break;
            }
            auto command = pending[processed++];
            switch (command.kind) {
            case CommandKind::AttachScript: {
                if (!current_registry->valid(command.entity)) {
                    break; // destroyed before its script attached
                }
                auto& script = current_registry->emplace<ScriptComponent>(command.entity, command.script);
                instance(command.entity, script, *current_assets);
                break;
            }
            case CommandKind::Destroy: {
                if (!current_registry->valid(command.entity)) {
                    break;
                }
                RelationComponent::detach(*current_registry, command.entity);
                std::vector<entt::entity> doomed { command.entity };
                for (auto& child : RelationComponent::get_children(*current_registry, command.entity, true)) {
                    doomed.push_back(child.first);
                }
                current_registry->destroy(doomed.begin(), doomed.end());
                break;
            }
            }
        }
        pending.clear();
    }

    sol::object find(std::string_view tag) override {
        require_registry("k2.find");
        auto entity = find_by_tag(*current_registry, tag);
        if (entity == entt::null) {
            return sol::lua_nil;
        }
        return sol::make_object(lua, make_handle(entity));
    }

    sol::table find_all(std::string_view tag) override {
        require_registry("k2.find_all");
        auto table = lua.create_table();
        for (auto entity : find_all_by_tag(*current_registry, tag)) {
            table.add(make_handle(entity));
        }
        return table;
    }

    sol::table entities(sol::variadic_args component_names) override {
        require_registry("k2.entities");
        std::vector<std::string> names;
        for (auto name : component_names) {
            names.push_back(name.get<std::string>());
        }
        auto table = lua.create_table();
        for (auto entity : find_with_components(*current_registry, names)) {
            table.add(make_handle(entity));
        }
        return table;
    }

    LuaEntity spawn(std::string_view tag, float x, float y) override {
        require_registry("k2.spawn");
        auto tmpl = find_by_tag(*current_registry, tag);
        if (tmpl == entt::null) {
            throw std::runtime_error(std::format("k2.spawn: no entity tagged '{}' to clone", tag));
        }
        return spawn_from(tmpl, x, y);
    }

    LuaEntity clone(const LuaEntity& source) override {
        require_registry("entity:clone");
        if (!source.valid()) {
            throw std::runtime_error("entity:clone called on an invalid entity");
        }
        auto entity = clone_entity(*current_registry, source.entity);
        copy_lua_component(source.entity, entity);
        defer_script_from(source.entity, entity);
        return make_handle(entity);
    }

    SceneView* scene_view() {
        auto* view = current_registry != nullptr ? current_registry->ctx().find<SceneView>() : nullptr;
        return (view != nullptr && view->viewport.w > 0.0f && view->viewport.h > 0.0f) ? view : nullptr;
    }

    std::tuple<float, float> screen_to_world(float x, float y) override {
        auto* view = scene_view();
        if (view == nullptr) {
            return { x, y };
        }
        auto world = view->camera.screen_to_world({ x, y }, view->viewport);
        return { world.x, world.y };
    }

    std::tuple<float, float> world_to_screen(float x, float y) override {
        auto* view = scene_view();
        if (view == nullptr) {
            return { x, y };
        }
        auto screen = view->camera.world_to_screen({ x, y }, view->viewport);
        return { screen.x, screen.y };
    }

    void play_sound(std::string_view name, sol::optional<float> volume, sol::optional<float> pitch) override {
        require_registry("k2.play_sound");
        if (!current_registry->ctx().contains<AudioSystem&>()
            || !current_registry->ctx().contains<ResourceManager&>()) {
            return;
        }
        auto& resources = current_registry->ctx().get<ResourceManager&>();
        const auto* clip = resources.try_get<AudioClip>(ResourceManager::resolve(name));
        if (clip == nullptr) {
            throw std::runtime_error(std::format("k2.play_sound: unknown clip '{}'", name));
        }
        current_registry->ctx().get<AudioSystem&>().play(*clip, volume.value_or(1.0f), pitch.value_or(1.0f));
    }

    void submit_draw(DrawCommand command) override {
        require_registry("k2.draw");
        auto& draw_list = current_registry->ctx().emplace<DrawList>();
        if (draw_list.commands.size() >= command_cap) {
            if (!draw_list.overflowed) {
                Log::core().warn("Draw command cap exceeded; dropping further primitives this frame");
                draw_list.overflowed = true;
            }
            return;
        }
        draw_list.commands.push_back(std::move(command));
    }

    void load_scene(std::string_view name) override {
        require_registry("k2.load_scene");
        auto it = current_assets->find(ResourceManager::resolve(name));
        if (it == current_assets->end() || it->second.second.type != Asset::Type::Scene) {
            throw std::runtime_error(std::format("k2.load_scene: unknown scene '{}'", name));
        }
        current_registry->ctx().insert_or_assign(SceneRequest { std::string { name } });
    }

    sol::table query_circle(float x, float y, float radius, sol::optional<std::uint32_t> mask) override {
        require_registry("k2.query_circle");
        auto table = lua.create_table();
        for (auto entity : collision::query_circle(*current_registry, { x, y }, radius, mask.value_or(0xffffffff))) {
            table.add(make_handle(entity));
        }
        return table;
    }

    sol::table query_aabb(float x, float y, float w, float h, sol::optional<std::uint32_t> mask) override {
        require_registry("k2.query_aabb");
        auto table = lua.create_table();
        for (auto entity :
            collision::query_aabb(*current_registry, { x, y }, { w * 0.5f, h * 0.5f }, mask.value_or(0xffffffff))) {
            table.add(make_handle(entity));
        }
        return table;
    }

    sol::table query_point(float x, float y, sol::optional<std::uint32_t> mask) override {
        require_registry("k2.query_point");
        auto table = lua.create_table();
        for (auto entity : collision::query_point(*current_registry, { x, y }, mask.value_or(0xffffffff))) {
            table.add(make_handle(entity));
        }
        return table;
    }

    bool overlaps(const LuaEntity& a, const LuaEntity& b) override {
        require_registry("entity:overlaps");
        if (!a.valid() || !b.valid()) {
            throw std::runtime_error("entity:overlaps called with an invalid entity");
        }
        return collision::overlaps(*current_registry, a.entity, b.entity);
    }

    void destroy(const LuaEntity& target) override {
        if (!current_registry || !target.valid()) {
            return;
        }
        pending.push_back({ .kind = CommandKind::Destroy, .entity = target.entity, .script = {} });
    }

    void require_registry(const char* who) const {
        if (!current_registry) {
            throw std::runtime_error(std::format("{} called outside a script hook", who));
        }
    }

    LuaEntity spawn_from(entt::entity tmpl, float x, float y) {
        auto entity = clone_entity(*current_registry, tmpl);
        copy_lua_component(tmpl, entity);
        auto& transform = current_registry->get_or_emplace<TransformComponent>(entity);
        transform.translation.x = x;
        transform.translation.y = y;
        defer_script_from(tmpl, entity);
        return make_handle(entity);
    }

    void copy_lua_component(entt::entity src, entt::entity dst) {
        const auto* component = current_registry->try_get<LuaComponent>(src);
        if (component != nullptr && component->valid()) {
            current_registry->emplace<LuaComponent>(dst, deep_copy_table(*component));
        }
    }

    void defer_script_from(entt::entity tmpl, entt::entity entity) {
        if (const auto* script = current_registry->try_get<ScriptComponent>(tmpl)) {
            pending.push_back({ .kind = CommandKind::AttachScript, .entity = entity, .script = script->script });
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
    impl->pending.clear();
    impl->reset_globals();
    ++*impl->epoch;
}

void ScriptSystem::update(Scene& scene, const AssetRegistry& assets, float dt) {
    auto& registry = scene.registry;
    impl->ensure_attached(registry);
    impl->current_registry = &registry;
    impl->current_assets = &assets;

    for (auto [entity, script] : registry.view<ScriptComponent>().each()) {
        auto& env = impl->instance(entity, script, assets);
        impl->call_hook(env, "on_update", script.script, impl->make_handle(entity), dt);
    }
    impl->flush_commands();
    impl->current_registry = nullptr;
    impl->current_assets = nullptr;
}

void ScriptSystem::fixed_update(Scene& scene, const AssetRegistry& assets, float dt) {
    auto& registry = scene.registry;
    impl->ensure_attached(registry);
    impl->current_registry = &registry;
    impl->current_assets = &assets;

    for (auto [entity, script] : registry.view<ScriptComponent>().each()) {
        auto& env = impl->instance(entity, script, assets);
        impl->call_hook(env, "on_fixed_update", script.script, impl->make_handle(entity), dt);
    }
    impl->flush_commands();
    impl->current_registry = nullptr;
    impl->current_assets = nullptr;
}

bool ScriptSystem::handle_event(Scene& scene, const AssetRegistry& assets, const Event* event) {
    auto translated = translate_event(impl->lua, event);
    if (!translated || (translated->input && !impl->input_enabled)) {
        return false;
    }
    auto& registry = scene.registry;
    impl->ensure_attached(registry);
    impl->current_registry = &registry;
    impl->current_assets = &assets;

    bool consumed = false;
    for (auto [entity, script] : registry.view<ScriptComponent>().each()) {
        auto& env = impl->instance(entity, script, assets);
        consumed |= impl->call_hook(env, "on_event", script.script, impl->make_handle(entity), translated->table);
    }
    impl->flush_commands();
    impl->current_registry = nullptr;
    impl->current_assets = nullptr;
    return consumed;
}

}
