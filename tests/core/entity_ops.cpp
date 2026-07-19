#include <catch2/catch_all.hpp>

#include "core/entity_ops.hpp"

#include "components/light.hpp"
#include "components/relation.hpp"
#include "components/script.hpp"
#include "components/sprite.hpp"
#include "components/tag.hpp"
#include "components/text.hpp"
#include "components/transform.hpp"

TEST_CASE("clone_entity copies the closed component set") {
    entt::registry registry;
    auto src = registry.create();
    registry.emplace<k2::TagComponent>(src, "hero");
    auto& transform = registry.emplace<k2::TransformComponent>(src);
    transform.translation = { 3.0f, 4.0f, 5.0f };
    registry.emplace<k2::SpriteComponent>(src);
    registry.emplace<k2::PointLight>(src).intensity = 2.5f;
    registry.emplace<k2::TextComponent>(src).text = "+1g";

    auto clone = k2::clone_entity(registry, src);

    REQUIRE(clone != src);
    REQUIRE(registry.all_of<k2::TagComponent>(clone));
    REQUIRE(registry.get<k2::TagComponent>(clone).tag == "hero");
    REQUIRE(registry.get<k2::TransformComponent>(clone).translation == glm::vec3 { 3.0f, 4.0f, 5.0f });
    REQUIRE(registry.all_of<k2::SpriteComponent>(clone));
    REQUIRE(registry.get<k2::PointLight>(clone).intensity == 2.5f);
    REQUIRE(registry.get<k2::TextComponent>(clone).text == "+1g");
}

TEST_CASE("clone_entity excludes Relation and Script") {
    entt::registry registry;
    auto parent = registry.create();
    auto src = registry.create();
    registry.emplace<k2::TagComponent>(src, "child");
    registry.emplace<k2::ScriptComponent>(src, k2::AssetHandle { "some_script" });
    auto& relation = registry.emplace<k2::RelationComponent>(src);
    relation.parent = parent;

    auto clone = k2::clone_entity(registry, src);

    REQUIRE(registry.all_of<k2::TagComponent>(clone));
    REQUIRE_FALSE(registry.all_of<k2::ScriptComponent>(clone));
    REQUIRE_FALSE(registry.all_of<k2::RelationComponent>(clone)); // a clone is a root
}

TEST_CASE("find_by_tag / find_all_by_tag match on tag") {
    entt::registry registry;
    auto a = registry.create();
    auto b = registry.create();
    auto c = registry.create();
    registry.emplace<k2::TagComponent>(a, "enemy");
    registry.emplace<k2::TagComponent>(b, "enemy");
    registry.emplace<k2::TagComponent>(c, "tower");

    auto found = k2::find_by_tag(registry, "enemy");
    REQUIRE((found == a || found == b));
    REQUIRE((k2::find_by_tag(registry, "missing") == entt::null));

    REQUIRE(k2::find_all_by_tag(registry, "enemy").size() == 2);
    REQUIRE(k2::find_all_by_tag(registry, "tower").size() == 1);
    REQUIRE(k2::find_all_by_tag(registry, "missing").empty());
}
