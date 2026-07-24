#include "core/script_system.hpp"

#include <cstdint>
#include <format>
#include <memory>
#include <string>
#include <vector>

#include <sol/sol.hpp>

#include "asset/loader.hpp"
#include "components/camera.hpp"
#include "components/relation.hpp"
#include "components/script.hpp"
#include "components/transform.hpp"
#include "core/audio_clip.hpp"
#include "core/audio_system.hpp"
#include "core/collision.hpp"
#include "core/entity_ops.hpp"
#include "core/logger.hpp"
#include "core/resources.hpp"
#include "core/runtime.hpp"
#include "core/scene.hpp"
#include "core/window.hpp"
#include "core/script/bindings.hpp"
#include "core/script/event_translation.hpp"
#include "core/script/host.hpp"
#include "core/script/lua_entity.hpp"
#include "core/script/scheduler.hpp"

namespace k2 {

struct ScriptSystemAttached { };

struct ScriptSystem::Impl : ScriptHost {
    sol::state lua;
    sol::protected_function scheduler_tick;
    Window* window {};
    bool input_enabled { true };

    entt::registry* attached_registry {};
    const AssetRegistry* attached_assets {};

    enum class CommandKind : std::uint8_t { AttachScript, Destroy };
    struct Command {
        CommandKind kind;
        entt::entity entity;
        AssetHandle script; // AttachScript only
    };
    std::vector<Command> pending;

    static constexpr std::size_t command_cap = 10000;

    explicit Impl(Window& window)
        : window { &window } {
        rebuild_vm();
    }

    void rebuild_vm() {
        scheduler_tick = sol::protected_function {}; // release the old VM's ref before destroying it
        lua = sol::state {};
        lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table, sol::lib::io,
            sol::lib::os, sol::lib::coroutine, sol::lib::package);
        bind_script_api(lua, *window, input_enabled, *this);
        scheduler_tick = lua.script(SCHEDULER_SOURCE);

        // require resolves modules through the asset registry
        lua["package"]["path"] = "";
        lua["package"]["cpath"] = "";
        lua["package"]["loadlib"] = sol::lua_nil;
        sol::object preload = lua["package"]["searchers"][1];
        lua["package"]["searchers"] = lua.create_table_with(1, preload, 2, [this](const std::string& module) {
            return search_module(module);
        });
    }

    // package searcher: a module is a Script asset, referenced by name
    sol::object search_module(const std::string& module) {
        auto fail = [&](const std::string& message) { return sol::make_object(lua, "\n\t" + message); };
        if (attached_assets == nullptr) {
            return fail("no asset registry attached");
        }
        auto it = attached_assets->find(ResourceManager::resolve(module));
        if (it == attached_assets->end() || it->second.second.type != Asset::Type::Script) {
            return fail(std::format("no Script asset named '{}'", module));
        }
        auto source = AssetLoader::try_get<Script>(it->second.second);
        if (!source) {
            return fail(std::format("asset '{}': {}", module, source.error()));
        }
        auto chunk = lua.load(source->source, "@" + module);
        if (!chunk.valid()) {
            sol::error err = chunk;
            return fail(err.what());
        }
        return sol::make_object(lua, chunk.get<sol::protected_function>());
    }

    LuaEntity make_handle(entt::entity entity) {
        return LuaEntity { .entity = entity, .registry = attached_registry };
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

    sol::environment& instance(entt::entity entity, ScriptComponent& script) {
        if (script.env) {
            return *script.env;
        }

        script.env = ScriptEnvironment { new sol::environment { lua, sol::create, lua.globals() },
            [](sol::environment* env) { delete env; } };
        auto& resources = attached_registry->ctx().get<Runtime&>().resources;
        const auto* source = resources.try_get<Script>(script.script.id);
        if (source == nullptr) {
            Log::core().warn(std::format("Scene references unknown script asset '{}'", script.script.name));
        } else if (!source->source.empty()) {
            auto result = lua.safe_script(source->source, *script.env, sol::script_pass_on_error, script.script.name);
            if (!result.valid()) {
                sol::error err = result;
                Log::core().error(std::format("Script '{}' failed to load: {}", script.script.name, err.what()));
            }
        }

        call_hook(*script.env, "on_create", script.script, make_handle(entity));
        return *script.env;
    }

    // Each scene runs in a fresh VM, so nothing a script did leaks into the next scene.
    void attach_scene(entt::registry& registry, const AssetRegistry& assets) {
        attached_registry = &registry;
        attached_assets = &assets;
        if (registry.ctx().contains<ScriptSystemAttached>()) {
            return;
        }
        registry.ctx().emplace<ScriptSystemAttached>();
        rebuild_vm();
        registry.on_destroy<ScriptComponent>().connect<&Impl::on_script_destroyed>(*this);
    }

    void on_script_destroyed(entt::registry& registry, entt::entity entity) {
        auto& script = registry.get<ScriptComponent>(entity);
        if (!script.env) {
            return;
        }
        call_hook(*script.env, "on_destroy", script.script, LuaEntity { .entity = entity, .registry = &registry });
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
                if (!attached_registry->valid(command.entity)) {
                    break; // destroyed before its script attached
                }
                auto& script = attached_registry->emplace<ScriptComponent>(command.entity, command.script);
                instance(command.entity, script);
                break;
            }
            case CommandKind::Destroy: {
                if (!attached_registry->valid(command.entity)) {
                    break;
                }
                destroy_with_children(*attached_registry, command.entity);
                break;
            }
            }
        }
        pending.clear();
    }

    sol::object find(std::string_view tag) override {
        require_registry("kione.find");
        auto entity = find_by_tag(*attached_registry, tag);
        if (entity == entt::null) {
            return sol::lua_nil;
        }
        return sol::make_object(lua, make_handle(entity));
    }

    sol::table find_all(std::string_view tag) override {
        require_registry("kione.find_all");
        auto table = lua.create_table();
        for (auto entity : find_all_by_tag(*attached_registry, tag)) {
            table.add(make_handle(entity));
        }
        return table;
    }

    sol::object find_main_camera() override {
        require_registry("kione.find_main_camera");
        for (auto entity : attached_registry->view<Camera, MainCamera>()) {
            return sol::make_object(lua, make_handle(entity));
        }
        return sol::lua_nil;
    }

    std::tuple<float, float> screen_size() override {
        if (auto* view = scene_view()) {
            return { view->viewport.w, view->viewport.h };
        }
        return { float(window->get_width()), float(window->get_height()) };
    }

    sol::table entities(sol::variadic_args component_names) override {
        require_registry("kione.entities");
        std::vector<std::string> names;
        for (auto name : component_names) {
            names.push_back(name.get<std::string>());
        }
        auto table = lua.create_table();
        for (auto entity : find_with_components(*attached_registry, names)) {
            table.add(make_handle(entity));
        }
        return table;
    }

    LuaEntity spawn(std::string_view tag, float x, float y) override {
        require_registry("kione.spawn");
        auto tmpl = find_by_tag(*attached_registry, tag);
        if (tmpl == entt::null) {
            throw std::runtime_error(std::format("kione.spawn: no entity tagged '{}' to clone", tag));
        }
        return spawn_from(tmpl, x, y);
    }

    LuaEntity clone(const LuaEntity& source) override {
        require_registry("entity:clone");
        if (!source.valid()) {
            throw std::runtime_error("entity:clone called on an invalid entity");
        }
        auto root = scene_root(*attached_registry);
        auto entity = clone_entity(*attached_registry, source.entity);
        RelationComponent::attach_last(*attached_registry, entity, root);
        defer_script_from(source.entity, entity);
        return make_handle(entity);
    }

    SceneView* scene_view() {
        auto* view = attached_registry != nullptr ? attached_registry->ctx().find<SceneView>() : nullptr;
        return (view != nullptr && view->viewport.w > 0.0f && view->viewport.h > 0.0f) ? view : nullptr;
    }

    std::tuple<float, float> screen_to_world(float x, float y) override {
        auto* view = scene_view();
        if (view == nullptr) {
            return { x, y };
        }
        auto world = view->screen_to_world({ x, y });
        return { world.x, world.y };
    }

    std::tuple<float, float> world_to_screen(float x, float y) override {
        auto* view = scene_view();
        if (view == nullptr) {
            return { x, y };
        }
        auto screen = view->world_to_screen({ x, y });
        return { screen.x, screen.y };
    }

    void play_sound(std::string_view name, sol::optional<float> volume, sol::optional<float> pitch) override {
        require_registry("kione.play_sound");
        auto& runtime = attached_registry->ctx().get<Runtime&>();
        const auto* clip = runtime.resources.try_get<AudioClip>(ResourceManager::resolve(name));
        if (clip == nullptr) {
            throw std::runtime_error(std::format("kione.play_sound: unknown clip '{}'", name));
        }
        runtime.audio.play(*clip, volume.value_or(1.0f), pitch.value_or(1.0f));
    }

    void submit_draw(DrawCommand command) override {
        require_registry("kione.draw");
        auto& draw_list = attached_registry->ctx().emplace<DrawList>();
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
        require_registry("kione.load_scene");
        auto it = attached_assets->find(ResourceManager::resolve(name));
        if (it == attached_assets->end() || it->second.second.type != Asset::Type::Scene) {
            throw std::runtime_error(std::format("kione.load_scene: unknown scene '{}'", name));
        }
        attached_registry->ctx().insert_or_assign(SceneRequest { std::string { name } });
    }

    sol::table query_circle(float x, float y, float radius, sol::optional<std::uint32_t> mask) override {
        require_registry("kione.query_circle");
        auto table = lua.create_table();
        for (auto entity : collision::query_circle(*attached_registry, { x, y }, radius, mask.value_or(0xffffffff))) {
            table.add(make_handle(entity));
        }
        return table;
    }

    sol::table query_aabb(float x, float y, float w, float h, sol::optional<std::uint32_t> mask) override {
        require_registry("kione.query_aabb");
        auto table = lua.create_table();
        for (auto entity :
            collision::query_aabb(*attached_registry, { x, y }, { w * 0.5f, h * 0.5f }, mask.value_or(0xffffffff))) {
            table.add(make_handle(entity));
        }
        return table;
    }

    sol::table query_point(float x, float y, sol::optional<std::uint32_t> mask) override {
        require_registry("kione.query_point");
        auto table = lua.create_table();
        for (auto entity : collision::query_point(*attached_registry, { x, y }, mask.value_or(0xffffffff))) {
            table.add(make_handle(entity));
        }
        return table;
    }

    bool overlaps(const LuaEntity& a, const LuaEntity& b) override {
        require_registry("entity:overlaps");
        if (!a.valid() || !b.valid()) {
            throw std::runtime_error("entity:overlaps called with an invalid entity");
        }
        return collision::overlaps(*attached_registry, a.entity, b.entity);
    }

    void destroy(const LuaEntity& target) override {
        if (!attached_registry || !target.valid()) {
            return;
        }
        pending.push_back({ .kind = CommandKind::Destroy, .entity = target.entity, .script = {} });
    }

    void require_registry(const char* who) const {
        if (!attached_registry) {
            throw std::runtime_error(std::format("{} called before a scene was attached", who));
        }
    }

    LuaEntity spawn_from(entt::entity tmpl, float x, float y) {
        auto root = scene_root(*attached_registry);
        auto entity = clone_entity(*attached_registry, tmpl);
        RelationComponent::attach_last(*attached_registry, entity, root);
        auto& transform = attached_registry->get_or_emplace<TransformComponent>(entity);
        transform.translation.x = x;
        transform.translation.y = y;
        defer_script_from(tmpl, entity);
        return make_handle(entity);
    }

    void defer_script_from(entt::entity tmpl, entt::entity entity) {
        defer_one_script(tmpl, entity);
        auto sources = RelationComponent::get_children(*attached_registry, tmpl, true);
        auto clones = RelationComponent::get_children(*attached_registry, entity, true);
        // the clone has same ring order as source
        for (std::size_t i = 0; i < sources.size(); i++) {
            defer_one_script(sources[i].first, clones[i].first);
        }
    }

    void defer_one_script(entt::entity tmpl, entt::entity entity) {
        if (const auto* script = attached_registry->try_get<ScriptComponent>(tmpl)) {
            pending.push_back({ .kind = CommandKind::AttachScript, .entity = entity, .script = script->script });
        }
    }
};

ScriptSystem::ScriptSystem(Window& window)
    : impl { std::make_unique<Impl>(window) } { }

ScriptSystem::~ScriptSystem() = default;

void ScriptSystem::set_input_enabled(bool enabled) { impl->input_enabled = enabled; }

void ScriptSystem::update(Scene& scene, const AssetRegistry& assets, float dt) {
    auto& registry = scene.registry;
    impl->attach_scene(registry, assets);

    if (auto ticked = impl->scheduler_tick(dt); !ticked.valid()) {
        sol::error err = ticked;
        Log::core().error(std::format("kione.after/tween tick: {}", err.what()));
    }

    for (auto [entity, script] : registry.view<ScriptComponent>().each()) {
        auto& env = impl->instance(entity, script);
        impl->call_hook(env, "on_update", script.script, impl->make_handle(entity), dt);
    }
    impl->flush_commands();
}

void ScriptSystem::fixed_update(Scene& scene, const AssetRegistry& assets, float dt) {
    auto& registry = scene.registry;
    impl->attach_scene(registry, assets);

    for (auto [entity, script] : registry.view<ScriptComponent>().each()) {
        auto& env = impl->instance(entity, script);
        impl->call_hook(env, "on_fixed_update", script.script, impl->make_handle(entity), dt);
    }
    impl->flush_commands();
}

bool ScriptSystem::handle_event(Scene& scene, const AssetRegistry& assets, const Event* event) {
    auto& registry = scene.registry;
    impl->attach_scene(registry, assets);
    auto translated = translate_event(impl->lua, event);
    if (!translated || (translated->input && !impl->input_enabled)) {
        return false;
    }

    bool consumed = false;
    for (auto [entity, script] : registry.view<ScriptComponent>().each()) {
        auto& env = impl->instance(entity, script);
        consumed |= impl->call_hook(env, "on_event", script.script, impl->make_handle(entity), translated->table);
    }
    impl->flush_commands();
    return consumed;
}

}
