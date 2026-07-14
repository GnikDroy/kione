#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <glm/gtc/constants.hpp>

#include "components/relation.hpp"
#include "components/transform.hpp"

using Catch::Approx;

namespace {
entt::entity make_entity(entt::registry& registry, glm::vec3 translation, glm::vec3 scale = { 1.0f, 1.0f, 1.0f },
    glm::quat orientation = glm::quat { 1.0f, 0.0f, 0.0f, 0.0f }) {
    auto entity = registry.create();
    registry.emplace<k2::TransformComponent>(entity, translation, orientation, scale);
    return entity;
}
}

TEST_CASE("world_transform of an entity without a parent is its local matrix", "[transforms]") {
    entt::registry registry;
    auto entity = make_entity(registry, { 10.0f, 20.0f, 3.0f });

    auto world = k2::TransformComponent::world(registry, entity);

    REQUIRE(world[3][0] == Approx(10.0f));
    REQUIRE(world[3][1] == Approx(20.0f));
    REQUIRE(world[3][2] == Approx(3.0f));
}

TEST_CASE("world_transform composes translation and scale down the parent chain", "[transforms]") {
    entt::registry registry;
    auto parent = make_entity(registry, { 100.0f, 0.0f, 0.0f }, { 2.0f, 2.0f, 1.0f });
    auto child = make_entity(registry, { 10.0f, 0.0f, 0.0f });
    k2::RelationComponent::attach_last(registry, child, parent);

    auto world = k2::TransformComponent::world(registry, child);

    REQUIRE(world[3][0] == Approx(120.0f));
    REQUIRE(world[3][1] == Approx(0.0f));
}

TEST_CASE("world_transform applies parent rotation to child offsets", "[transforms]") {
    entt::registry registry;
    auto quarter_turn = glm::angleAxis(glm::half_pi<float>(), glm::vec3 { 0.0f, 0.0f, 1.0f });
    auto parent = make_entity(registry, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, quarter_turn);
    auto child = make_entity(registry, { 10.0f, 0.0f, 0.0f });
    k2::RelationComponent::attach_last(registry, child, parent);

    auto world = k2::TransformComponent::world(registry, child);

    REQUIRE(world[3][0] == Approx(0.0f).margin(1e-5));
    REQUIRE(world[3][1] == Approx(10.0f));
}

TEST_CASE("world_transform composes across three levels", "[transforms]") {
    entt::registry registry;
    auto root = make_entity(registry, { 1.0f, 0.0f, 0.0f });
    auto mid = make_entity(registry, { 2.0f, 0.0f, 0.0f });
    auto leaf = make_entity(registry, { 4.0f, 0.0f, 0.0f });
    k2::RelationComponent::attach_last(registry, mid, root);
    k2::RelationComponent::attach_last(registry, leaf, mid);

    auto world = k2::TransformComponent::world(registry, leaf);

    REQUIRE(world[3][0] == Approx(7.0f));
}

TEST_CASE("world_transform skips ancestors without a transform", "[transforms]") {
    entt::registry registry;
    auto group = registry.create();
    auto child = make_entity(registry, { 5.0f, 0.0f, 0.0f });
    k2::RelationComponent::attach_last(registry, child, group);

    auto world = k2::TransformComponent::world(registry, child);

    REQUIRE(world[3][0] == Approx(5.0f));
}

TEST_CASE("parent_world_transform is identity for root entities", "[transforms]") {
    entt::registry registry;
    auto entity = make_entity(registry, { 10.0f, 20.0f, 0.0f });

    REQUIRE(k2::TransformComponent::parent_world(registry, entity) == glm::mat4 { 1.0f });
}

TEST_CASE("set_from_matrix round-trips a TRS matrix", "[transforms]") {
    k2::TransformComponent source {
        .translation { 3.0f, -4.0f, 5.0f },
        .orientation = glm::angleAxis(0.7f, glm::vec3 { 0.0f, 0.0f, 1.0f }),
        .scale { 2.0f, 3.0f, 1.0f },
    };

    k2::TransformComponent result {};
    result.set_from_matrix(source.get_matrix());

    REQUIRE(result.translation.x == Approx(source.translation.x));
    REQUIRE(result.translation.y == Approx(source.translation.y));
    REQUIRE(result.translation.z == Approx(source.translation.z));
    REQUIRE(result.scale.x == Approx(source.scale.x));
    REQUIRE(result.scale.y == Approx(source.scale.y));
    REQUIRE(result.scale.z == Approx(source.scale.z));
    REQUIRE(std::abs(glm::dot(result.orientation, source.orientation)) == Approx(1.0f));
}

TEST_CASE("reparenting math preserves world placement", "[transforms]") {
    entt::registry registry;
    auto parent = make_entity(registry, { 100.0f, 50.0f, 0.0f }, { 2.0f, 2.0f, 1.0f },
        glm::angleAxis(glm::half_pi<float>(), glm::vec3 { 0.0f, 0.0f, 1.0f }));
    auto entity = make_entity(registry, { 10.0f, 20.0f, 1.0f });

    auto world_before = k2::TransformComponent::world(registry, entity);
    k2::RelationComponent::attach_last(registry, entity, parent);
    auto& transform = registry.get<k2::TransformComponent>(entity);
    transform.set_from_matrix(glm::inverse(k2::TransformComponent::parent_world(registry, entity)) * world_before);
    auto world_after = k2::TransformComponent::world(registry, entity);

    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            REQUIRE(world_after[col][row] == Approx(world_before[col][row]).margin(1e-4));
        }
    }
}

TEST_CASE("world_transform terminates on hierarchy cycles", "[transforms]") {
    entt::registry registry;
    auto a = make_entity(registry, { 1.0f, 0.0f, 0.0f });
    auto b = make_entity(registry, { 1.0f, 0.0f, 0.0f });
    registry.emplace<k2::RelationComponent>(a, k2::RelationComponent { .parent = b });
    registry.emplace<k2::RelationComponent>(b, k2::RelationComponent { .parent = a });

    static_cast<void>(k2::TransformComponent::world(registry, a));
    SUCCEED();
}
