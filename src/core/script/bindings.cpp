#include "core/script/bindings.hpp"

#include <string>

#include <glm/glm.hpp>

#include "core/logger.hpp"
#include "core/script/key_names.hpp"
#include "core/script/lua_entity.hpp"
#include "core/window.hpp"

namespace k2 {

void bind_script_api(sol::state& lua, Window& window, const bool& input_enabled) {
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
            [](SpriteComponent& sprite, const std::string& name) { sprite.texture.set(name); }));

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
        &LuaEntity::spot_light, "ambient_light", &LuaEntity::ambient_light, "sprite_light", &LuaEntity::sprite_light,
        "tag", &LuaEntity::tag, "valid", &LuaEntity::valid);

    auto k2_table = lua.create_named_table("k2");
    k2_table["log"] = [](const std::string& message) { Log::app().info(message); };

    auto input = lua.create_named_table("Input");
    input["is_key_down"] = [&window, &input_enabled](const std::string& key) {
        auto code = key_code_from(key);
        return code && input_enabled && window.keyboard.get_state(*code) == KeyboardDevice::KeyState::press;
    };
}

}
