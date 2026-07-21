#include <catch2/catch_all.hpp>

#include "core/script/lua_entity.hpp"

#include <memory>

#include "components/sprite.hpp"
#include "components/transform.hpp"

static k2::LuaEntity handle_for(
    entt::registry& registry, entt::entity entity, const std::shared_ptr<const void>& token) {
    return k2::LuaEntity { .entity = entity, .registry = &registry, .scene_token = token };
}

static std::shared_ptr<const void> make_token() { return std::make_shared<const bool>(true); }

TEST_CASE("A live handle resolves components") {
    entt::registry registry;
    auto token = make_token();
    auto entity = registry.create();
    registry.emplace<k2::TransformComponent>(entity);
    registry.emplace<k2::SpriteComponent>(entity);
    registry.emplace<k2::Camera>(entity);

    auto handle = handle_for(registry, entity, token);
    REQUIRE(handle.valid());
    REQUIRE(handle.transform() != nullptr);
    REQUIRE(handle.sprite() != nullptr);
    REQUIRE(handle.camera() != nullptr);
    REQUIRE(handle.point_light() == nullptr); // absent component
    REQUIRE(handle.id() == entt::to_integral(entity));
}

TEST_CASE("tag is readable and writable through a live handle") {
    entt::registry registry;
    auto token = make_token();
    auto entity = registry.create();
    registry.emplace<k2::TagComponent>(entity, "fx");
    auto handle = handle_for(registry, entity, token);

    REQUIRE(handle.tag() == "fx");
    handle.set_tag("fx_spawned");
    REQUIRE(handle.tag() == "fx_spawned");
    REQUIRE(registry.get<k2::TagComponent>(entity).tag == "fx_spawned");

    registry.destroy(entity);
    handle.set_tag("zombie"); // invalid handle: a silent no-op, like the other accessors
    REQUIRE(handle.tag().empty());
}

TEST_CASE("A destroyed entity's handle is invalid and yields nil accessors") {
    entt::registry registry;
    auto token = make_token();
    auto entity = registry.create();
    registry.emplace<k2::TransformComponent>(entity);
    auto handle = handle_for(registry, entity, token);

    registry.destroy(entity);

    REQUIRE_FALSE(handle.valid());
    REQUIRE(handle.transform() == nullptr);
}

TEST_CASE("A recycled slot does not resurrect an old handle") {
    entt::registry registry;
    auto token = make_token();
    auto first = registry.create();
    auto stale = handle_for(registry, first, token);
    registry.destroy(first);
    auto reused = registry.create(); // same index, bumped version
    registry.emplace<k2::TransformComponent>(reused);

    REQUIRE_FALSE(stale.valid()); // version mismatch
    REQUIRE(handle_for(registry, reused, token).valid());
}

TEST_CASE("An expired scene token invalidates a handle before touching the registry") {
    entt::registry registry;
    auto token = make_token();
    auto entity = registry.create();
    auto handle = handle_for(registry, entity, token);
    REQUIRE(handle.valid());

    token.reset(); // the owning scene's script state died

    REQUIRE_FALSE(handle.valid());
    REQUIRE(handle.transform() == nullptr);
}

TEST_CASE("A null-token handle is invalid") {
    entt::registry registry;
    auto entity = registry.create();
    k2::LuaEntity handle { .entity = entity, .registry = &registry, .scene_token = {} };
    REQUIRE_FALSE(handle.valid());
}
