#include "core/script/bindings.hpp"

#include <format>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "core/logger.hpp"
#include "core/script/host.hpp"
#include "core/script/key_names.hpp"
#include "core/script/lua_entity.hpp"
#include "core/window.hpp"

namespace k2 {
namespace {

    // A read-only Lua table: so every read of an unknown key, and every write raise an error.
    void set_strict_constants(sol::state& lua, const std::string& name, const std::vector<std::string>& values) {
        auto backing = lua.create_table();
        for (const auto& value : values) {
            backing.raw_set(value, value);
        }

        auto meta = lua.create_table();
        meta["__index"] = [name, backing](const sol::table&, const std::string& key) -> std::string {
            auto value = backing.raw_get<sol::optional<std::string>>(key);
            if (!value) {
                throw std::runtime_error(std::format("Unknown {} constant '{}'", name, key));
            }
            return *value;
        };
        meta["__newindex"] = [name](const sol::table&, const sol::object&, const sol::object&) {
            throw std::runtime_error(std::format("{} is a read-only constant table", name));
        };
        meta["__metatable"] = false; // disable getmetatable/setmetatable

        auto table = lua.create_named_table(name);
        table[sol::metatable_key] = meta;
    }

}

std::unordered_set<std::string> table_string_keys(const sol::table& table) {
    std::unordered_set<std::string> keys;
    for (const auto& [key, value] : table) {
        if (key.is<std::string>()) {
            keys.insert(key.as<std::string>());
        }
    }
    return keys;
}

void reset_table_to_baseline(sol::table table, const std::unordered_set<std::string>& baseline) {
    std::vector<std::string> extra;
    for (const auto& [key, value] : table) {
        if (key.is<std::string>() && !baseline.contains(key.as<std::string>())) {
            extra.push_back(key.as<std::string>());
        }
    }
    for (const auto& key : extra) {
        table[key] = sol::lua_nil;
    }
}

void bind_constants(sol::state& lua) {
    set_strict_constants(lua, "Key", key_names_all());
    set_strict_constants(lua, "MouseButton", { "left", "right", "middle" });
    set_strict_constants(lua, "InputState", { "press", "release", "repeat" });
    set_strict_constants(lua, "EventType",
        { "key", "char", "mouse_button", "mouse_drop", "cursor_position", "cursor_enter", "scroll", "window_close",
            "window_resize", "framebuffer_resize", "content_scale", "window_reposition", "window_iconify",
            "window_maximize", "window_focus" });
}

void bind_script_api(sol::state& lua, Window& window, const bool& input_enabled, ScriptHost& host) {
    lua.new_usertype<glm::vec3>("vec3", sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(), "x",
        &glm::vec3::x, "y", &glm::vec3::y, "z", &glm::vec3::z);
    lua.new_usertype<glm::vec4>("vec4", sol::constructors<glm::vec4(), glm::vec4(float, float, float, float)>(), "x",
        &glm::vec4::x, "y", &glm::vec4::y, "z", &glm::vec4::z, "w", &glm::vec4::w);

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

    lua.new_usertype<Rectf>("Rect", "x", &Rectf::x, "y", &Rectf::y, "w", &Rectf::w, "h", &Rectf::h);

    lua.new_usertype<SpriteComponent>("Sprite", "color",
        sol::property([](SpriteComponent& sprite) -> glm::vec4& { return sprite.color; },
            [](SpriteComponent& sprite, const glm::vec4& value) { sprite.color = value; }),
        "uv",
        sol::property([](SpriteComponent& sprite) -> Rectf& { return sprite.uv_rect; },
            [](SpriteComponent& sprite, const Rectf& value) { sprite.uv_rect = value; }),
        "texture",
        sol::property([](SpriteComponent& sprite) { return sprite.texture.name; },
            [](SpriteComponent& sprite, const std::string& name) { sprite.texture.set(name); }),
        "unlit", &SpriteComponent::unlit);

    auto color_property = []<class Light>() {
        return sol::property([](Light& light) -> glm::vec3& { return light.color; },
            [](Light& light, const glm::vec3& value) { light.color = value; });
    };

    lua.new_usertype<PointLight>("PointLight", "color", color_property.template operator()<PointLight>(), "intensity",
        &PointLight::intensity, "radius", &PointLight::radius);

    lua.new_usertype<SpotLight>("SpotLight", "color", color_property.template operator()<SpotLight>(), "intensity",
        &SpotLight::intensity, "radius", &SpotLight::radius, "inner_angle", &SpotLight::inner_angle, "outer_angle",
        &SpotLight::outer_angle);

    lua.new_usertype<AmbientLight>("AmbientLight", "color", color_property.template operator()<AmbientLight>(),
        "intensity", &AmbientLight::intensity);

    lua.new_usertype<SpriteLight>("SpriteLight", "color", color_property.template operator()<SpriteLight>(),
        "intensity", &SpriteLight::intensity, "texture",
        sol::property([](SpriteLight& light) { return light.texture.name; },
            [](SpriteLight& light, const std::string& name) { light.texture.set(name); }));

    lua.new_usertype<AnimationComponent>(
        "Animation", "playing", &AnimationComponent::playing, "speed", &AnimationComponent::speed, "finished",
        sol::property([](AnimationComponent& animation) { return animation.finished; }), "clip",
        sol::property([](AnimationComponent& animation) { return animation.clip.name; }), "play",
        [](AnimationComponent& animation, const std::string& clip) {
            animation.clip.set(clip);
            animation.elapsed = 0.0f;
            animation.playing = true;
            animation.finished = false;
        },
        "stop", [](AnimationComponent& animation) { animation.playing = false; });

    lua.new_usertype<LuaEntity>(
        "Entity", "transform", &LuaEntity::transform, "sprite", &LuaEntity::sprite, "animation", &LuaEntity::animation,
        "point_light", &LuaEntity::point_light, "spot_light", &LuaEntity::spot_light, "ambient_light",
        &LuaEntity::ambient_light, "sprite_light", &LuaEntity::sprite_light, "tag", &LuaEntity::tag, "valid",
        &LuaEntity::valid, "id", &LuaEntity::id, "clone", [&host](const LuaEntity& self) { return host.clone(self); },
        "destroy", [&host](const LuaEntity& self) { host.destroy(self); }, sol::meta_function::equal_to,
        &LuaEntity::operator==);

    bind_constants(lua);

    auto k2_table = lua.create_named_table("k2");
    k2_table["log"] = [](const std::string& message) { Log::app().info(message); };
    k2_table["find"] = [&host](std::string_view tag) { return host.find(tag); };
    k2_table["find_all"] = [&host](std::string_view tag) { return host.find_all(tag); };
    k2_table["spawn"] = [&host](std::string_view tag, float x, float y) { return host.spawn(tag, x, y); };

    auto input = lua.create_named_table("Input");
    input["is_key_down"] = [&window, &input_enabled](const std::string& key) {
        auto code = key_code_from(key);
        return code && input_enabled && window.keyboard.get_state(*code) == KeyboardDevice::KeyState::press;
    };
}

}
