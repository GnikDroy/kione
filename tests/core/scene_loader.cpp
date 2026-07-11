#include <catch2/catch_all.hpp>

#include "core/scene_loader.hpp"

using namespace k2::literals;

namespace {
entt::entity find_by_tag(entt::registry& registry, std::string_view tag) {
    for (auto [entity, tag_component] : registry.view<k2::TagComponent>().each()) {
        if (tag_component.tag == tag) {
            return entity;
        }
    }
    return entt::null;
}
}

TEST_CASE("SceneLoader round trip remaps entity references") {
    k2::Scene original;
    auto& registry = original.registry;

    // Create id gaps so a fresh registry cannot reproduce the serialized ids.
    auto e0 = registry.create();
    auto e1 = registry.create();
    auto e2 = registry.create();
    auto e3 = registry.create();
    auto parent = registry.create();
    registry.destroy(e0);
    registry.destroy(e1);
    registry.destroy(e2);
    registry.destroy(e3);

    registry.emplace<k2::TagComponent>(parent, "parent");
    registry.emplace<k2::TransformComponent>(parent).scale = { 2.0f, 2.0f, 1.0f };

    auto child_a = registry.create();
    registry.emplace<k2::TagComponent>(child_a, "child_a");
    registry.emplace<k2::SpriteComponent>(child_a).texture = k2::AssetHandle { "cobble" };
    k2::RelationComponent::attach_last(registry, child_a, parent);

    auto child_b = registry.create();
    registry.emplace<k2::TagComponent>(child_b, "child_b");
    k2::RelationComponent::attach_last(registry, child_b, parent);

    k2::ResourceManager resources;
    k2::AssetRegistry assets;
    auto loaded = k2::SceneLoader::load(YAML::Node { original }, resources, assets);
    auto& loaded_registry = loaded.registry;

    REQUIRE(loaded_registry.ctx().contains<k2::ResourceManager&>());

    auto new_parent = find_by_tag(loaded_registry, "parent");
    auto new_child_a = find_by_tag(loaded_registry, "child_a");
    auto new_child_b = find_by_tag(loaded_registry, "child_b");
    REQUIRE((new_parent != entt::null));
    REQUIRE((new_child_a != entt::null));
    REQUIRE((new_child_b != entt::null));

    // The fresh registry allocated different ids, so equality would mean no remapping happened.
    REQUIRE((new_parent != parent));

    auto& parent_relation = loaded_registry.get<k2::RelationComponent>(new_parent);
    REQUIRE((parent_relation.parent == entt::null));
    REQUIRE(parent_relation.children == 2);
    REQUIRE((parent_relation.first == new_child_a));

    auto& child_a_relation = loaded_registry.get<k2::RelationComponent>(new_child_a);
    REQUIRE((child_a_relation.parent == new_parent));
    REQUIRE((child_a_relation.next == new_child_b));

    auto& child_b_relation = loaded_registry.get<k2::RelationComponent>(new_child_b);
    REQUIRE((child_b_relation.parent == new_parent));
    REQUIRE((child_b_relation.prev == new_child_a));

    auto& sprite = loaded_registry.get<k2::SpriteComponent>(new_child_a);
    REQUIRE(sprite.texture.name == "cobble");
    REQUIRE(sprite.texture.id == "cobble"_fnv1a);

    REQUIRE(loaded_registry.get<k2::TransformComponent>(new_parent).scale.x == 2.0f);
}

TEST_CASE("SceneLoader rejects malformed scenes") {
    k2::ResourceManager resources;
    k2::AssetRegistry assets;
    REQUIRE_THROWS_AS(k2::SceneLoader::load(YAML::Load("not a sequence"), resources, assets), std::runtime_error);
}
