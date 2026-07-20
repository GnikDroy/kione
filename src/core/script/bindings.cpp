#include "core/script/bindings.hpp"

#include <format>
#include <string>
#include <tuple>
#include <vector>

#include <glm/glm.hpp>

#include "core/logger.hpp"
#include "core/resources.hpp"
#include "core/script/host.hpp"
#include "core/script/lua_component.hpp"
#include "core/script/key_names.hpp"
#include "core/script/lua_entity.hpp"
#include "core/window.hpp"
#include "rendering/font.hpp"

namespace k2 {
namespace {

    DrawCommand parse_draw_options(sol::optional<sol::table> options, DrawCommand command) {
        if (!options) {
            return command;
        }
        auto& table = *options;
        if (auto color = table.get<sol::optional<sol::object>>("color")) {
            if (color->is<glm::vec4>()) {
                command.color = color->as<glm::vec4>();
            } else if (color->is<sol::table>()) {
                auto channels = color->as<sol::table>();
                command.color = { channels.get_or(1, 1.0f), channels.get_or(2, 1.0f), channels.get_or(3, 1.0f),
                    channels.get_or(4, 1.0f) };
            }
        }
        command.z = table.get_or("z", command.z);
        command.width = table.get_or("width", table.get_or("thickness", table.get_or("size", command.width)));
        command.filled = table.get_or("filled", command.filled);
        command.unlit = table.get_or("unlit", command.unlit);
        command.closed = table.get_or("closed", command.closed);
        command.segments = table.get_or("segments", command.segments);
        return command;
    }

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

    lua.new_usertype<Camera>("Camera", "position",
        sol::property([](Camera& camera) -> glm::vec3& { return camera.position; },
            [](Camera& camera, const glm::vec3& value) { camera.position = value; }),
        "target",
        sol::property([](Camera& camera) -> glm::vec3& { return camera.target; },
            [](Camera& camera, const glm::vec3& value) { camera.target = value; }),
        "look_at", [](Camera& camera, float x, float y) {
            camera.position.x = x;
            camera.position.y = y;
            camera.target.x = x;
            camera.target.y = y;
        });

    lua.new_usertype<SpriteComponent>("Sprite", "color",
        sol::property([](SpriteComponent& sprite) -> glm::vec4& { return sprite.color; },
            [](SpriteComponent& sprite, const glm::vec4& value) { sprite.color = value; }),
        "uv",
        sol::property([](SpriteComponent& sprite) -> Rectf& { return sprite.uv_rect; },
            [](SpriteComponent& sprite, const Rectf& value) { sprite.uv_rect = value; }),
        "texture",
        sol::property([](SpriteComponent& sprite) { return sprite.texture.name; },
            [](SpriteComponent& sprite, const std::string& name) { sprite.texture.set(name); }),
        "unlit", &SpriteComponent::unlit, "blend",
        sol::property(
            [](SpriteComponent& sprite) { return sprite.blend == BlendMode::Additive ? "additive" : "alpha"; },
            [](SpriteComponent& sprite, std::string_view value) {
                if (value == "alpha") {
                    sprite.blend = BlendMode::Alpha;
                } else if (value == "additive") {
                    sprite.blend = BlendMode::Additive;
                } else {
                    throw std::runtime_error(std::format("Unknown blend mode '{}'", value));
                }
            }),
        "intensity", &SpriteComponent::intensity);

    auto color_property = []<class Light>() {
        return sol::property([](Light& light) -> glm::vec3& { return light.color; },
            [](Light& light, const glm::vec3& value) { light.color = value; });
    };

    lua.new_usertype<PointLight>("PointLight", "color", color_property.template operator()<PointLight>(), "intensity",
        &PointLight::intensity, "radius", &PointLight::radius);

    lua.new_usertype<SpotLight>("SpotLight", "color", color_property.template operator()<SpotLight>(), "intensity",
        &SpotLight::intensity, "radius", &SpotLight::radius, "inner_angle", &SpotLight::inner_angle, "outer_angle",
        &SpotLight::outer_angle);

    lua.new_usertype<SpriteLight>("SpriteLight", "color", color_property.template operator()<SpriteLight>(),
        "intensity", &SpriteLight::intensity, "texture",
        sol::property([](SpriteLight& light) { return light.texture.name; },
            [](SpriteLight& light, const std::string& name) { light.texture.set(name); }));

    lua.new_usertype<TextComponent>("Text", "text", &TextComponent::text, "size", &TextComponent::size, "color",
        sol::property([](TextComponent& text) -> glm::vec4& { return text.color; },
            [](TextComponent& text, const glm::vec4& value) { text.color = value; }),
        "font",
        sol::property([](TextComponent& text) { return text.font.name; },
            [](TextComponent& text, const std::string& name) { text.font.set(name); }));

    lua.new_usertype<AudioSourceComponent>("AudioSource", "volume", &AudioSourceComponent::volume, "pitch",
        &AudioSourceComponent::pitch, "looping", &AudioSourceComponent::looping, "clip",
        sol::property([](AudioSourceComponent& source) { return source.clip.name; },
            [](AudioSourceComponent& source, const std::string& name) { source.clip.set(name); }));

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

    lua.new_usertype<ColliderComponent>("Collider", "shape",
        sol::property(
            [](ColliderComponent& collider) -> const char* {
                if (std::holds_alternative<BoxShape>(collider.shape)) {
                    return "box";
                }
                return std::holds_alternative<CircleShape>(collider.shape) ? "circle" : "pill";
            },
            [](ColliderComponent& collider, std::string_view value) {
                if (value == "box") {
                    collider.shape = BoxShape {};
                } else if (value == "circle") {
                    collider.shape = CircleShape {};
                } else if (value == "pill") {
                    collider.shape = PillShape {};
                } else {
                    throw std::runtime_error(std::format("Unknown collider shape '{}'", value));
                }
            }),
        "radius",
        sol::property(
            [](ColliderComponent& collider) -> float {
                if (auto* circle = std::get_if<CircleShape>(&collider.shape)) {
                    return circle->radius;
                }
                if (auto* pill = std::get_if<PillShape>(&collider.shape)) {
                    return pill->radius;
                }
                throw std::runtime_error("collider.radius: a box collider has no radius");
            },
            [](ColliderComponent& collider, float value) {
                if (auto* circle = std::get_if<CircleShape>(&collider.shape)) {
                    circle->radius = value;
                } else if (auto* pill = std::get_if<PillShape>(&collider.shape)) {
                    pill->radius = value;
                } else {
                    throw std::runtime_error("collider.radius: a box collider has no radius");
                }
            }),
        "half_height",
        sol::property(
            [](ColliderComponent& collider) -> float {
                if (auto* pill = std::get_if<PillShape>(&collider.shape)) {
                    return pill->half_height;
                }
                throw std::runtime_error("collider.half_height: only pill colliders have a half_height");
            },
            [](ColliderComponent& collider, float value) {
                if (auto* pill = std::get_if<PillShape>(&collider.shape)) {
                    pill->half_height = value;
                } else {
                    throw std::runtime_error("collider.half_height: only pill colliders have a half_height");
                }
            }),
        "width",
        sol::property(
            [](ColliderComponent& collider) -> float {
                if (auto* box = std::get_if<BoxShape>(&collider.shape)) {
                    return box->size.x;
                }
                throw std::runtime_error("collider.width: only box colliders have a width");
            },
            [](ColliderComponent& collider, float value) {
                if (auto* box = std::get_if<BoxShape>(&collider.shape)) {
                    box->size.x = value;
                } else {
                    throw std::runtime_error("collider.width: only box colliders have a width");
                }
            }),
        "height",
        sol::property(
            [](ColliderComponent& collider) -> float {
                if (auto* box = std::get_if<BoxShape>(&collider.shape)) {
                    return box->size.y;
                }
                throw std::runtime_error("collider.height: only box colliders have a height");
            },
            [](ColliderComponent& collider, float value) {
                if (auto* box = std::get_if<BoxShape>(&collider.shape)) {
                    box->size.y = value;
                } else {
                    throw std::runtime_error("collider.height: only box colliders have a height");
                }
            }),
        "layer", &ColliderComponent::layer, "mask", &ColliderComponent::mask);

    lua.new_usertype<Environment>("Environment", "ambient_color",
        sol::property([](Environment& env) -> glm::vec3& { return env.ambient_color; },
            [](Environment& env, const glm::vec3& value) { env.ambient_color = value; }),
        "ambient_intensity", &Environment::ambient_intensity, "clear_color",
        sol::property([](Environment& env) -> glm::vec4& { return env.clear_color; },
            [](Environment& env, const glm::vec4& value) { env.clear_color = value; }),
        "bloom", &Environment::bloom, "bloom_intensity", &Environment::bloom_intensity, "bloom_threshold",
        &Environment::bloom_threshold);

    lua.new_usertype<LuaEntity>(
        "Entity", "transform", &LuaEntity::transform, "sprite", &LuaEntity::sprite, "text", &LuaEntity::text,
        "text_size",
        [](const LuaEntity& self) -> std::tuple<float, float> {
            auto* text = self.text();
            if (text == nullptr || !self.registry->ctx().contains<ResourceManager&>()) {
                return { 0.0f, 0.0f };
            }
            auto* font = self.registry->ctx().get<ResourceManager&>().try_get<Font>(text->font.id);
            if (font == nullptr) {
                return { 0.0f, 0.0f };
            }
            auto metrics = font->measure(text->text, text->size);
            return { metrics.width, metrics.height };
        },
        "animation", &LuaEntity::animation, "audio_source", &LuaEntity::audio_source,
        "collider", &LuaEntity::collider, "environment", &LuaEntity::environment, "overlaps",
        [&host](const LuaEntity& self, const LuaEntity& other) { return host.overlaps(self, other); },
        "point_light", &LuaEntity::point_light, "spot_light", &LuaEntity::spot_light, "sprite_light", &LuaEntity::sprite_light, "data",
        [&lua](const LuaEntity& self) -> sol::object {
            if (!self.valid()) {
                return sol::lua_nil;
            }
            return lua_component(lua, *self.registry, self.entity);
        },
        "camera", &LuaEntity::camera, "tag",
        sol::property([](const LuaEntity& self) { return self.tag(); },
            [](const LuaEntity& self, const std::string& value) { self.set_tag(value); }),
        "valid",
        &LuaEntity::valid, "id", &LuaEntity::id, "clone", [&host](const LuaEntity& self) { return host.clone(self); },
        "destroy", [&host](const LuaEntity& self) { host.destroy(self); }, sol::meta_function::equal_to,
        &LuaEntity::operator==);

    bind_constants(lua);

    auto kione_table = lua.create_named_table("kione");
    kione_table["log"] = [](const std::string& message) { Log::app().info(message); };
    kione_table["find"] = [&host](std::string_view tag) { return host.find(tag); };
    kione_table["find_all"] = [&host](std::string_view tag) { return host.find_all(tag); };
    kione_table["entities"] = [&host](sol::variadic_args component_names) { return host.entities(component_names); };
    kione_table["spawn"] = [&host](std::string_view tag, float x, float y) { return host.spawn(tag, x, y); };
    kione_table["screen_to_world"] = [&host](float x, float y) { return host.screen_to_world(x, y); };
    kione_table["world_to_screen"] = [&host](float x, float y) { return host.world_to_screen(x, y); };
    kione_table["play_sound"] = [&host](std::string_view name, sol::optional<float> volume, sol::optional<float> pitch) {
        host.play_sound(name, volume, pitch);
    };
    kione_table["load_scene"] = [&host](std::string_view name) { host.load_scene(name); };
    kione_table["query_circle"] = [&host](float x, float y, float radius, sol::optional<std::uint32_t> mask) {
        return host.query_circle(x, y, radius, mask);
    };
    kione_table["query_aabb"] = [&host](float x, float y, float w, float h, sol::optional<std::uint32_t> mask) {
        return host.query_aabb(x, y, w, h, mask);
    };
    kione_table["query_point"]
        = [&host](float x, float y, sol::optional<std::uint32_t> mask) { return host.query_point(x, y, mask); };
    kione_table["draw_line"] = [&host](float x1, float y1, float x2, float y2, sol::optional<sol::table> options) {
        host.submit_draw(parse_draw_options(
            options, DrawCommand { .kind = DrawCommand::Kind::Line, .a = { x1, y1 }, .b = { x2, y2 } }));
    };
    kione_table["draw_rect"] = [&host](float x, float y, float w, float h, sol::optional<sol::table> options) {
        host.submit_draw(parse_draw_options(
            options, DrawCommand { .kind = DrawCommand::Kind::Rect, .a = { x, y }, .b = { w, h } }));
    };
    kione_table["draw_circle"] = [&host](float x, float y, float radius, sol::optional<sol::table> options) {
        host.submit_draw(parse_draw_options(options,
            DrawCommand { .kind = DrawCommand::Kind::Circle, .a = { x, y }, .radius = radius, .width = 2.0f }));
    };
    kione_table["draw_point"] = [&host](float x, float y, sol::optional<sol::table> options) {
        host.submit_draw(parse_draw_options(
            options, DrawCommand { .kind = DrawCommand::Kind::Point, .a = { x, y }, .width = 4.0f }));
    };
    kione_table["draw_polygon"] = [&host](sol::table flat_points, sol::optional<sol::table> options) {
        DrawCommand command { .kind = DrawCommand::Kind::Polygon };
        for (std::size_t i = 1; i + 1 <= flat_points.size(); i += 2) {
            command.points.push_back({ flat_points.get<float>(i), flat_points.get<float>(i + 1) });
        }
        host.submit_draw(parse_draw_options(options, std::move(command)));
    };

    auto input = lua.create_named_table("Input");
    input["is_key_down"] = [&window, &input_enabled](const std::string& key) {
        auto code = key_code_from(key);
        return code && input_enabled && window.keyboard.get_state(*code) == KeyboardDevice::KeyState::press;
    };
}

}
