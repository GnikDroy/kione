#include "core/script_system.hpp"

#include <format>
#include <string>
#include <unordered_map>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "asset/scheme.hpp"
#include "components/animation.hpp"
#include "components/light.hpp"
#include "components/script.hpp"
#include "components/sprite.hpp"
#include "components/tag.hpp"
#include "components/transform.hpp"
#include "core/keyboard.hpp"
#include "core/logger.hpp"
#include "core/scene.hpp"
#include "core/window.hpp"

namespace k2 {
namespace {

    struct LuaEntity {
        entt::entity entity { entt::null };
        entt::registry* registry {};

        [[nodiscard]] TransformComponent* transform() const { return registry->try_get<TransformComponent>(entity); }
        [[nodiscard]] SpriteComponent* sprite() const { return registry->try_get<SpriteComponent>(entity); }
        [[nodiscard]] AnimationComponent* animation() const { return registry->try_get<AnimationComponent>(entity); }
        [[nodiscard]] PointLight* point_light() const { return registry->try_get<PointLight>(entity); }
        [[nodiscard]] SpotLight* spot_light() const { return registry->try_get<SpotLight>(entity); }
        [[nodiscard]] AmbientLight* ambient_light() const { return registry->try_get<AmbientLight>(entity); }
        [[nodiscard]] SpriteLight* sprite_light() const { return registry->try_get<SpriteLight>(entity); }
        [[nodiscard]] std::string tag() const {
            auto* tag_component = registry->try_get<TagComponent>(entity);
            return tag_component ? tag_component->tag : std::string {};
        }
        [[nodiscard]] bool valid() const { return registry->valid(entity); }
    };

    KeyboardDevice::KeyCode key_code_from(const std::string& key) {
        using KeyCode = KeyboardDevice::KeyCode;
        static const std::unordered_map<std::string, KeyCode> named {
            { "space", KeyCode::key_space },
            { "left", KeyCode::key_left },
            { "right", KeyCode::key_right },
            { "up", KeyCode::key_up },
            { "down", KeyCode::key_down },
        };

        if (key.size() == 1 && key[0] >= 'a' && key[0] <= 'z') {
            return static_cast<KeyCode>(int(KeyCode::key_a) + (key[0] - 'a'));
        }
        if (auto it = named.find(key); it != named.end()) {
            return it->second;
        }
        throw std::invalid_argument(std::format("Unknown key name '{}'", key));
    }

}

struct ScriptSystem::Impl {
    Window& window;
    sol::state lua;
    std::unordered_map<entt::entity, sol::environment> instances;
    std::unordered_map<ResourceID, std::string> sources;
    entt::registry* attached {};
    bool input_enabled { true };

    explicit Impl(Window& window)
        : window { window } {
        lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table);
        bind_api();
    }

    void bind_api() {
        lua.new_usertype<glm::vec3>("vec3", sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(), "x",
            &glm::vec3::x, "y", &glm::vec3::y, "z", &glm::vec3::z);
        lua.new_usertype<glm::vec4>("vec4", sol::constructors<glm::vec4(), glm::vec4(float, float, float, float)>(),
            "x", &glm::vec4::x, "y", &glm::vec4::y, "z", &glm::vec4::z, "w", &glm::vec4::w);

        lua.new_usertype<TransformComponent>("Transform", "translation",
            sol::property([](TransformComponent& transform) -> glm::vec3& { return transform.translation; },
                [](TransformComponent& transform, const glm::vec3& value) { transform.translation = value; }),
            "scale",
            sol::property([](TransformComponent& transform) -> glm::vec3& { return transform.scale; },
                [](TransformComponent& transform, const glm::vec3& value) { transform.scale = value; }),
            "angle",
            sol::property([](TransformComponent& transform) { return glm::eulerAngles(transform.orientation).z; },
                [](TransformComponent& transform, float angle) {
                    transform.orientation = glm::quat(glm::vec3 { 0.0f, 0.0f, angle });
                }));

        lua.new_usertype<Rectf>(
            "Rect", "x", &Rectf::x, "y", &Rectf::y, "w", &Rectf::w, "h", &Rectf::h);

        lua.new_usertype<SpriteComponent>("Sprite", "color",
            sol::property([](SpriteComponent& sprite) -> glm::vec4& { return sprite.color; },
                [](SpriteComponent& sprite, const glm::vec4& value) { sprite.color = value; }),
            "uv",
            sol::property([](SpriteComponent& sprite) -> Rectf& { return sprite.uv_rect; },
                [](SpriteComponent& sprite, const Rectf& value) { sprite.uv_rect = value; }),
            "texture",
            sol::property([](SpriteComponent& sprite) { return sprite.texture.name; },
                [](SpriteComponent& sprite, const std::string& name) { sprite.texture.set(name); }));

        auto color_property = []<class Light>() {
            return sol::property([](Light& light) -> glm::vec3& { return light.color; },
                [](Light& light, const glm::vec3& value) { light.color = value; });
        };

        lua.new_usertype<PointLight>("PointLight", "color", color_property.template operator()<PointLight>(),
            "intensity", &PointLight::intensity, "radius", &PointLight::radius);

        lua.new_usertype<SpotLight>("SpotLight", "color", color_property.template operator()<SpotLight>(),
            "intensity", &SpotLight::intensity, "radius", &SpotLight::radius, "inner_angle", &SpotLight::inner_angle,
            "outer_angle", &SpotLight::outer_angle);

        lua.new_usertype<AmbientLight>("AmbientLight", "color", color_property.template operator()<AmbientLight>(),
            "intensity", &AmbientLight::intensity);

        lua.new_usertype<SpriteLight>("SpriteLight", "color", color_property.template operator()<SpriteLight>(),
            "intensity", &SpriteLight::intensity, "texture",
            sol::property([](SpriteLight& light) { return light.texture.name; },
                [](SpriteLight& light, const std::string& name) { light.texture.set(name); }));

        lua.new_usertype<AnimationComponent>("Animation", "playing", &AnimationComponent::playing, "speed",
            &AnimationComponent::speed, "finished",
            sol::property([](AnimationComponent& animation) { return animation.finished; }), "clip",
            sol::property([](AnimationComponent& animation) { return animation.clip.name; }), "play",
            [](AnimationComponent& animation, const std::string& clip) {
                animation.clip.set(clip);
                animation.elapsed = 0.0f;
                animation.playing = true;
                animation.finished = false;
            },
            "stop", [](AnimationComponent& animation) { animation.playing = false; });

        lua.new_usertype<LuaEntity>("Entity", "transform", &LuaEntity::transform, "sprite", &LuaEntity::sprite,
            "animation", &LuaEntity::animation, "point_light", &LuaEntity::point_light, "spot_light",
            &LuaEntity::spot_light, "ambient_light", &LuaEntity::ambient_light, "sprite_light",
            &LuaEntity::sprite_light, "tag", &LuaEntity::tag, "valid", &LuaEntity::valid);

        auto k2_table = lua.create_named_table("k2");
        k2_table["log"] = [](const std::string& message) { Log::app().info(message); };

        auto input = lua.create_named_table("Input");
        input["is_key_down"] = [this](const std::string& key) {
            return input_enabled && window.keyboard.get_state(key_code_from(key)) == KeyboardDevice::KeyState::press;
        };
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
    void call_hook(sol::environment& env, const char* hook, const AssetHandle& script, Args&&... args) {
        sol::protected_function function = env[hook];
        if (!function.valid()) {
            return;
        }
        auto result = function(std::forward<Args>(args)...);
        if (!result.valid()) {
            sol::error err = result;
            Log::core().error(std::format("Script '{}' {}: {}", script.name, hook, err.what()));
            env[hook] = sol::lua_nil;
        }
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
    if (impl->attached != &registry) {
        impl->instances.clear();
        registry.on_destroy<ScriptComponent>().connect<&Impl::on_script_destroyed>(*impl);
        impl->attached = &registry;
    }

    for (auto [entity, script] : registry.view<ScriptComponent>().each()) {
        auto& env = impl->instance(registry, entity, script, assets);
        impl->call_hook(env, "on_update", script.script, LuaEntity { .entity = entity, .registry = &registry }, dt);
    }
}

}
