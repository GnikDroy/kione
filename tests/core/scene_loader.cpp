#include <catch2/catch_all.hpp>

#include "core/scene_loader.hpp"

#include "components/relation.hpp"
#include "serializers/core/scene.hpp"

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
    registry.emplace<k2::MainCamera>(parent);

    auto child_a = registry.create();
    registry.emplace<k2::TagComponent>(child_a, "child_a");
    registry.emplace<k2::SpriteComponent>(child_a).texture = k2::AssetHandle { "cobble" };
    k2::RelationComponent::attach_last(registry, child_a, parent);

    auto child_b = registry.create();
    registry.emplace<k2::TagComponent>(child_b, "child_b");
    registry.emplace<k2::PointLight>(child_b, glm::vec3 { 1.0f, 0.5f, 0.25f }, 2.0f, 640.0f);
    registry.emplace<k2::Environment>(child_b).ambient_intensity = 0.5f;
    k2::RelationComponent::attach_last(registry, child_b, parent);

    k2::ResourceManager resources;
    k2::AssetRegistry assets;
    auto result = k2::SceneLoader::load(YAML::Node { original }, resources, assets);
    REQUIRE(result.has_value());
    auto& loaded_registry = result->registry;

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
    REQUIRE(loaded_registry.all_of<k2::MainCamera>(new_parent));

    auto& point_light = loaded_registry.get<k2::PointLight>(new_child_b);
    REQUIRE(point_light.color.x == 1.0f);
    REQUIRE(point_light.intensity == 2.0f);
    REQUIRE(point_light.radius == 640.0f);
    REQUIRE(loaded_registry.get<k2::Environment>(new_child_b).ambient_intensity == 0.5f);
}

TEST_CASE("SceneLoader rejects malformed scenes") {
    k2::ResourceManager resources;
    k2::AssetRegistry assets;
    REQUIRE_FALSE(k2::SceneLoader::load(YAML::Load("not a sequence"), resources, assets).has_value());
}

TEST_CASE("SceneLoader loads an empty scene") {
    k2::ResourceManager resources;
    k2::AssetRegistry assets;
    auto loaded = k2::SceneLoader::load(YAML::Node { k2::Scene {} }, resources, assets);
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->registry.view<entt::entity>().size() == 0);
}

TEST_CASE("SceneLoader round trips script, camera, and remaining light components") {
    k2::Scene original;
    auto& registry = original.registry;

    auto entity = registry.create();
    registry.emplace<k2::TagComponent>(entity, "loaded");
    registry.emplace<k2::ScriptComponent>(entity, k2::AssetHandle { "mover" });
    registry.emplace<k2::SpotLight>(entity, glm::vec3 { 0.0f, 1.0f, 0.0f }, 3.0f, 400.0f, 0.2f, 0.5f);
    registry.emplace<k2::SpriteLight>(entity, k2::AssetHandle { "glow" }, glm::vec3 { 1.0f, 0.0f, 1.0f }, 1.5f);
    registry.emplace<k2::Camera>(entity).projection_traits = k2::Camera::OrthographicTraits {
        .left = -100.0f,
        .right = 100.0f,
        .top = 50.0f,
        .bottom = -50.0f,
        .far_clip = 0.0f,
        .near_clip = 2000.0f,
    };

    k2::ResourceManager resources;
    k2::AssetRegistry assets;
    auto loaded = k2::SceneLoader::load(YAML::Node { original }, resources, assets);
    REQUIRE(loaded.has_value());
    auto& loaded_registry = loaded->registry;

    auto new_entity = find_by_tag(loaded_registry, "loaded");
    REQUIRE((new_entity != entt::null));

    REQUIRE(loaded_registry.get<k2::ScriptComponent>(new_entity).script.name == "mover");

    auto& spot = loaded_registry.get<k2::SpotLight>(new_entity);
    REQUIRE(spot.radius == 400.0f);
    REQUIRE(spot.inner_angle == 0.2f);
    REQUIRE(spot.outer_angle == 0.5f);

    auto& sprite_light = loaded_registry.get<k2::SpriteLight>(new_entity);
    REQUIRE(sprite_light.texture.name == "glow");
    REQUIRE(sprite_light.intensity == 1.5f);

    auto* traits = std::get_if<k2::Camera::OrthographicTraits>(
        &loaded_registry.get<k2::Camera>(new_entity).projection_traits);
    REQUIRE(traits != nullptr);
    REQUIRE(traits->near_clip == 2000.0f);
}

TEST_CASE("SceneLoader remaps grandchildren across the whole subtree") {
    k2::Scene original;
    auto& registry = original.registry;

    // Force id gaps so the loaded registry cannot reuse the serialized ids.
    for (int i = 0; i < 5; i++) {
        registry.destroy(registry.create());
    }

    auto root = registry.create();
    auto child = registry.create();
    auto grandchild = registry.create();
    registry.emplace<k2::TagComponent>(root, "root");
    registry.emplace<k2::TagComponent>(child, "child");
    registry.emplace<k2::TagComponent>(grandchild, "grandchild");
    k2::RelationComponent::attach_last(registry, child, root);
    k2::RelationComponent::attach_last(registry, grandchild, child);

    k2::ResourceManager resources;
    k2::AssetRegistry assets;
    auto loaded = k2::SceneLoader::load(YAML::Node { original }, resources, assets);
    REQUIRE(loaded.has_value());
    auto& loaded_registry = loaded->registry;

    auto new_root = find_by_tag(loaded_registry, "root");
    auto new_child = find_by_tag(loaded_registry, "child");
    auto new_grandchild = find_by_tag(loaded_registry, "grandchild");

    REQUIRE((loaded_registry.get<k2::RelationComponent>(new_child).parent == new_root));
    REQUIRE((loaded_registry.get<k2::RelationComponent>(new_grandchild).parent == new_child));
    REQUIRE((loaded_registry.get<k2::RelationComponent>(new_child).first == new_grandchild));
}
