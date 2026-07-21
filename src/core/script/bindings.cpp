#include "core/script/bindings.hpp"

#include <format>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <glm/glm.hpp>

#include "core/logger.hpp"
#include "core/resources.hpp"
#include "core/script/host.hpp"
#include "core/script/key_names.hpp"
#include "core/script/lua_component.hpp"
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

    template <class Owner, class Member> auto ref_property(Member Owner::* member) {
        return sol::property([member](Owner& owner) -> Member& { return owner.*member; },
            [member](Owner& owner, const Member& value) { owner.*member = value; });
    }

    template <class Owner> auto asset_property(AssetHandle Owner::* member) {
        return sol::property([member](Owner& owner) { return (owner.*member).name; },
            [member](Owner& owner, const std::string& name) { (owner.*member).set(name); });
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
    auto vec2 = lua.new_usertype<glm::vec2>("vec2", sol::constructors<glm::vec2(), glm::vec2(float, float)>());
    vec2["x"] = &glm::vec2::x;
    vec2["y"] = &glm::vec2::y;

    auto vec3 = lua.new_usertype<glm::vec3>("vec3", sol::constructors<glm::vec3(), glm::vec3(float, float, float)>());
    vec3["x"] = &glm::vec3::x;
    vec3["y"] = &glm::vec3::y;
    vec3["z"] = &glm::vec3::z;

    auto vec4
        = lua.new_usertype<glm::vec4>("vec4", sol::constructors<glm::vec4(), glm::vec4(float, float, float, float)>());
    vec4["x"] = &glm::vec4::x;
    vec4["y"] = &glm::vec4::y;
    vec4["z"] = &glm::vec4::z;
    vec4["w"] = &glm::vec4::w;

    auto rect = lua.new_usertype<Rectf>("Rect");
    rect["x"] = &Rectf::x;
    rect["y"] = &Rectf::y;
    rect["w"] = &Rectf::w;
    rect["h"] = &Rectf::h;

    auto transform = lua.new_usertype<TransformComponent>("Transform");
    transform["translation"] = ref_property(&TransformComponent::translation);
    transform["scale"] = ref_property(&TransformComponent::scale);
    transform["angle"]
        = sol::property([](TransformComponent& transform) { return glm::eulerAngles(transform.orientation).z; },
            [](TransformComponent& transform, float angle) {
                transform.orientation = glm::quat(glm::vec3 { 0.0f, 0.0f, angle });
            });

    auto camera = lua.new_usertype<Camera>("Camera");
    camera["position"] = ref_property(&Camera::position);
    camera["target"] = ref_property(&Camera::target);
    camera["look_at"] = [](Camera& camera, float x, float y) {
        camera.position.x = x;
        camera.position.y = y;
        camera.target.x = x;
        camera.target.y = y;
    };

    auto sprite = lua.new_usertype<SpriteComponent>("Sprite");
    sprite["color"] = ref_property(&SpriteComponent::color);
    sprite["uv"] = ref_property(&SpriteComponent::uv_rect);
    sprite["texture"] = asset_property(&SpriteComponent::texture);
    sprite["unlit"] = &SpriteComponent::unlit;
    sprite["intensity"] = &SpriteComponent::intensity;
    sprite["blend"] = sol::property(
        [](SpriteComponent& sprite) { return sprite.blend == BlendMode::Additive ? "additive" : "alpha"; },
        [](SpriteComponent& sprite, std::string_view value) {
            if (value == "alpha") {
                sprite.blend = BlendMode::Alpha;
            } else if (value == "additive") {
                sprite.blend = BlendMode::Additive;
            } else {
                throw std::runtime_error(std::format("Unknown blend mode '{}'", value));
            }
        });

    auto point_light = lua.new_usertype<PointLight>("PointLight");
    point_light["color"] = ref_property(&PointLight::color);
    point_light["intensity"] = &PointLight::intensity;
    point_light["radius"] = &PointLight::radius;

    auto spot_light = lua.new_usertype<SpotLight>("SpotLight");
    spot_light["color"] = ref_property(&SpotLight::color);
    spot_light["intensity"] = &SpotLight::intensity;
    spot_light["radius"] = &SpotLight::radius;
    spot_light["inner_angle"] = &SpotLight::inner_angle;
    spot_light["outer_angle"] = &SpotLight::outer_angle;

    auto sprite_light = lua.new_usertype<SpriteLight>("SpriteLight");
    sprite_light["color"] = ref_property(&SpriteLight::color);
    sprite_light["intensity"] = &SpriteLight::intensity;
    sprite_light["texture"] = asset_property(&SpriteLight::texture);

    auto text = lua.new_usertype<TextComponent>("Text");
    text["text"] = &TextComponent::text;
    text["size"] = &TextComponent::size;
    text["color"] = ref_property(&TextComponent::color);
    text["font"] = asset_property(&TextComponent::font);

    auto audio_source = lua.new_usertype<AudioSourceComponent>("AudioSource");
    audio_source["volume"] = &AudioSourceComponent::volume;
    audio_source["pitch"] = &AudioSourceComponent::pitch;
    audio_source["looping"] = &AudioSourceComponent::looping;
    audio_source["clip"] = asset_property(&AudioSourceComponent::clip);

    auto animation = lua.new_usertype<AnimationComponent>("Animation");
    animation["playing"] = &AnimationComponent::playing;
    animation["speed"] = &AnimationComponent::speed;
    animation["finished"] = sol::property([](AnimationComponent& animation) { return animation.finished; });
    animation["clip"] = sol::property([](AnimationComponent& animation) { return animation.clip.name; });
    animation["play"] = [](AnimationComponent& animation, const std::string& clip) {
        animation.clip.set(clip);
        animation.elapsed = 0.0f;
        animation.playing = true;
        animation.finished = false;
    };
    animation["stop"] = [](AnimationComponent& animation) { animation.playing = false; };

    auto tilemap = lua.new_usertype<TileMapComponent>("TileMap");
    tilemap["width"] = sol::property([](TileMapComponent& tilemap) { return tilemap.size.x; });
    tilemap["height"] = sol::property([](TileMapComponent& tilemap) { return tilemap.size.y; });
    tilemap["tile_size"] = ref_property(&TileMapComponent::tile_size);
    tilemap["color"] = ref_property(&TileMapComponent::color);
    tilemap["unlit"] = &TileMapComponent::unlit;
    tilemap["tileset"] = asset_property(&TileMapComponent::tileset);
    // Scripts see empty cells as nil; out-of-bounds access raises a script error.
    tilemap["get_tile"] = [&lua](TileMapComponent& tilemap, int x, int y) -> sol::object {
        auto tile = tilemap[x, y];
        return tile == TileMapComponent::empty_tile ? sol::lua_nil : sol::make_object(lua, int(tile));
    };
    tilemap["set_tile"] = [](TileMapComponent& tilemap, int x, int y, sol::optional<int> index) {
        auto value = index.value_or(-1);
        tilemap[x, y] = value < 0 || value > int(TileMapComponent::empty_tile) ? TileMapComponent::empty_tile
                                                                               : std::uint16_t(value);
    };

    auto collider = lua.new_usertype<ColliderComponent>("Collider");
    collider["layer"] = &ColliderComponent::layer;
    collider["mask"] = &ColliderComponent::mask;
    collider["shape"] = sol::property(
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
        });
    collider["radius"] = sol::property(
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
        });
    collider["half_height"] = sol::property(
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
        });
    collider["width"] = sol::property(
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
        });
    collider["height"] = sol::property(
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
        });

    auto environment = lua.new_usertype<Environment>("Environment");
    environment["ambient_color"] = ref_property(&Environment::ambient_color);
    environment["ambient_intensity"] = &Environment::ambient_intensity;
    environment["clear_color"] = ref_property(&Environment::clear_color);
    environment["bloom"] = &Environment::bloom;
    environment["bloom_intensity"] = &Environment::bloom_intensity;
    environment["bloom_threshold"] = &Environment::bloom_threshold;

    auto entity = lua.new_usertype<LuaEntity>("Entity");
    entity["transform"] = &LuaEntity::transform;
    entity["sprite"] = &LuaEntity::sprite;
    entity["text"] = &LuaEntity::text;
    entity["animation"] = &LuaEntity::animation;
    entity["tilemap"] = &LuaEntity::tilemap;
    entity["audio_source"] = &LuaEntity::audio_source;
    entity["collider"] = &LuaEntity::collider;
    entity["environment"] = &LuaEntity::environment;
    entity["point_light"] = &LuaEntity::point_light;
    entity["spot_light"] = &LuaEntity::spot_light;
    entity["sprite_light"] = &LuaEntity::sprite_light;
    entity["camera"] = &LuaEntity::camera;
    entity["valid"] = &LuaEntity::valid;
    entity["id"] = &LuaEntity::id;
    entity["tag"] = sol::property([](const LuaEntity& self) { return self.tag(); },
        [](const LuaEntity& self, const std::string& value) { self.set_tag(value); });
    entity["text_size"] = [](const LuaEntity& self) -> std::tuple<float, float> {
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
    };
    entity["data"] = [&lua](const LuaEntity& self) -> sol::object {
        if (!self.valid()) {
            return sol::lua_nil;
        }
        return lua_component(lua, *self.registry, self.entity);
    };
    entity["overlaps"] = [&host](const LuaEntity& self, const LuaEntity& other) { return host.overlaps(self, other); };
    entity["clone"] = [&host](const LuaEntity& self) { return host.clone(self); };
    entity["destroy"] = [&host](const LuaEntity& self) { host.destroy(self); };
    entity[sol::meta_function::equal_to] = &LuaEntity::operator==;

    bind_constants(lua);

    auto kione_table = lua.create_named_table("kione");
    kione_table["log"] = [](const std::string& message) { Log::app().info(message); };
    kione_table["find"] = [&host](std::string_view tag) { return host.find(tag); };
    kione_table["find_all"] = [&host](std::string_view tag) { return host.find_all(tag); };
    kione_table["entities"] = [&host](sol::variadic_args component_names) { return host.entities(component_names); };
    kione_table["spawn"] = [&host](std::string_view tag, float x, float y) { return host.spawn(tag, x, y); };
    kione_table["screen_to_world"] = [&host](float x, float y) { return host.screen_to_world(x, y); };
    kione_table["world_to_screen"] = [&host](float x, float y) { return host.world_to_screen(x, y); };
    kione_table["play_sound"] = [&host](std::string_view name, sol::optional<float> volume,
                                    sol::optional<float> pitch) { host.play_sound(name, volume, pitch); };
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
            std::move(options), DrawCommand { .kind = DrawCommand::Kind::Line, .a = { x1, y1 }, .b = { x2, y2 } }));
    };
    kione_table["draw_rect"] = [&host](float x, float y, float w, float h, sol::optional<sol::table> options) {
        host.submit_draw(
            parse_draw_options(std::move(options), DrawCommand { .kind = DrawCommand::Kind::Rect, .a = { x, y }, .b = { w, h } }));
    };
    kione_table["draw_circle"] = [&host](float x, float y, float radius, sol::optional<sol::table> options) {
        host.submit_draw(parse_draw_options(std::move(options),
            DrawCommand { .kind = DrawCommand::Kind::Circle, .a = { x, y }, .radius = radius, .width = 2.0f }));
    };
    kione_table["draw_point"] = [&host](float x, float y, sol::optional<sol::table> options) {
        host.submit_draw(parse_draw_options(
            std::move(options), DrawCommand { .kind = DrawCommand::Kind::Point, .a = { x, y }, .width = 4.0f }));
    };
    kione_table["draw_polygon"] = [&host](const sol::table& flat_points, sol::optional<sol::table> options) {
        DrawCommand command { .kind = DrawCommand::Kind::Polygon };
        for (std::size_t i = 1; i + 1 <= flat_points.size(); i += 2) {
            command.points.emplace_back(flat_points.get<float>(i), flat_points.get<float>(i + 1));
        }
        host.submit_draw(parse_draw_options(std::move(options), std::move(command)));
    };

    auto input = lua.create_named_table("Input");
    input["is_key_down"] = [&window, &input_enabled](const std::string& key) {
        auto code = key_code_from(key);
        return code && input_enabled && window.keyboard.get_state(*code) == KeyboardDevice::KeyState::press;
    };
}

}
