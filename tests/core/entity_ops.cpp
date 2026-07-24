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

TEST_CASE("clone_entity deep-copies children in ring order") {
    entt::registry registry;
    auto src = registry.create();
    registry.emplace<k2::TagComponent>(src, "tank");
    auto turret = registry.create();
    registry.emplace<k2::TagComponent>(turret, "turret");
    registry.emplace<k2::SpriteComponent>(turret);
    auto antenna = registry.create();
    registry.emplace<k2::TagComponent>(antenna, "antenna");
    k2::RelationComponent::attach_last(registry, turret, src);
    k2::RelationComponent::attach_last(registry, antenna, src);
    auto muzzle = registry.create();
    registry.emplace<k2::TagComponent>(muzzle, "muzzle");
    k2::RelationComponent::attach_last(registry, muzzle, turret);

    auto clone = k2::clone_entity(registry, src);

    REQUIRE((registry.get<k2::RelationComponent>(clone).parent == entt::null)); // still a root
    auto children = k2::RelationComponent::get_children(registry, clone);
    REQUIRE(children.size() == 2);
    REQUIRE(children[0].first != turret);
    REQUIRE(registry.get<k2::TagComponent>(children[0].first).tag == "turret");
    REQUIRE(registry.all_of<k2::SpriteComponent>(children[0].first));
    REQUIRE(registry.get<k2::TagComponent>(children[1].first).tag == "antenna");
    auto grandchildren = k2::RelationComponent::get_children(registry, children[0].first);
    REQUIRE(grandchildren.size() == 1);
    REQUIRE(registry.get<k2::TagComponent>(grandchildren.front().first).tag == "muzzle");
    // the source hierarchy is untouched
    REQUIRE(k2::RelationComponent::get_children(registry, src).size() == 2);
}

TEST_CASE("scene_root is created once and reused") {
    entt::registry registry;
    auto root = k2::scene_root(registry);
    REQUIRE((registry.get<k2::RelationComponent>(root).parent == entt::null));
    REQUIRE(registry.all_of<k2::TransformComponent>(root));
    REQUIRE(k2::scene_root(registry) == root);
}

TEST_CASE("create_entity appends under the scene root in order") {
    entt::registry registry;
    auto a = k2::create_entity(registry);
    auto b = k2::create_entity(registry);

    auto roots = k2::RelationComponent::get_children(registry, k2::scene_root(registry));
    REQUIRE(roots.size() == 2);
    REQUIRE(roots[0].first == a);
    REQUIRE(roots[1].first == b);
    REQUIRE(registry.all_of<k2::TransformComponent>(a));
}

TEST_CASE("find_with_components filters by named components") {
    entt::registry registry;
    auto lit_sprite = registry.create();
    registry.emplace<k2::SpriteComponent>(lit_sprite);
    registry.emplace<k2::PointLight>(lit_sprite);
    auto bare_sprite = registry.create();
    registry.emplace<k2::SpriteComponent>(bare_sprite);
    [[maybe_unused]] auto empty = registry.create(); // component-less, still counts as an entity

    REQUIRE(k2::find_with_components(registry, {}).size() == 3); // no filter: every entity
    REQUIRE(k2::find_with_components(registry, std::vector<std::string> { "Sprite" }).size() == 2);

    auto lit = k2::find_with_components(registry, std::vector<std::string> { "Sprite", "PointLight" });
    REQUIRE(lit.size() == 1);
    REQUIRE(lit.front() == lit_sprite);

    REQUIRE(k2::find_with_components(registry, std::vector<std::string> { "Camera" }).empty());
    REQUIRE_THROWS(k2::find_with_components(registry, std::vector<std::string> { "Sprit" })); // typo fails loudly
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
