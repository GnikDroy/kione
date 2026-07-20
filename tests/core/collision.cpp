#include <catch2/catch_all.hpp>

#include "core/collision.hpp"

#include <glm/gtc/constants.hpp>

using k2::BoxShape;
using k2::CircleShape;
using k2::ColliderComponent;
using k2::PillShape;
using k2::TransformComponent;
using k2::collision::overlaps_maskless;
using k2::collision::WorldCollider;

namespace {

WorldCollider circle_at(glm::vec2 center, float radius) {
    return { .collider = { .shape = CircleShape { .radius = radius } }, .center = center, .angle = 0.0f };
}

WorldCollider box_at(glm::vec2 center, glm::vec2 size, float angle = 0.0f) {
    return { .collider = { .shape = BoxShape { .size = size } }, .center = center, .angle = angle };
}

WorldCollider pill_at(glm::vec2 center, float radius, float half_height, float angle = 0.0f) {
    return { .collider = { .shape = PillShape { .radius = radius, .half_height = half_height } }, .center = center, .angle = angle };
}

}

TEST_CASE("circle vs circle") {
    REQUIRE(overlaps_maskless(circle_at({ 0, 0 }, 10), circle_at({ 19, 0 }, 10)));
    REQUIRE(overlaps_maskless(circle_at({ 0, 0 }, 10), circle_at({ 20, 0 }, 10))); // touching counts
    REQUIRE_FALSE(overlaps_maskless(circle_at({ 0, 0 }, 10), circle_at({ 21, 0 }, 10)));
    REQUIRE(overlaps_maskless(circle_at({ 0, 0 }, 10), circle_at({ 0, 0 }, 1))); // contained
}

TEST_CASE("circle vs box") {
    auto box = box_at({ 0, 0 }, { 40, 20 });
    REQUIRE(overlaps_maskless(circle_at({ 29, 0 }, 10), box));
    REQUIRE(overlaps_maskless(circle_at({ 30, 0 }, 10), box)); // touching the right edge
    REQUIRE_FALSE(overlaps_maskless(circle_at({ 31, 0 }, 10), box));
    REQUIRE(overlaps_maskless(circle_at({ 0, 0 }, 1), box)); // center inside
    REQUIRE_FALSE(overlaps_maskless(circle_at({ 25, 15 }, 5), box)); // corner diagonal miss
}

TEST_CASE("rotated box vs circle") {
    // A long thin box rotated 90 degrees swaps which circles it reaches.
    auto quarter_turn = glm::half_pi<float>();
    auto flat = box_at({ 0, 0 }, { 40, 4 });
    auto upright = box_at({ 0, 0 }, { 40, 4 }, quarter_turn);
    REQUIRE(overlaps_maskless(circle_at({ 15, 0 }, 5), flat));
    REQUIRE_FALSE(overlaps_maskless(circle_at({ 15, 0 }, 5), upright));
    REQUIRE_FALSE(overlaps_maskless(circle_at({ 0, 15 }, 5), flat));
    REQUIRE(overlaps_maskless(circle_at({ 0, 15 }, 5), upright));
}

TEST_CASE("box vs box") {
    REQUIRE(overlaps_maskless(box_at({ 0, 0 }, { 20, 20 }), box_at({ 19, 0 }, { 20, 20 })));
    REQUIRE(overlaps_maskless(box_at({ 0, 0 }, { 20, 20 }), box_at({ 20, 0 }, { 20, 20 }))); // touching
    REQUIRE_FALSE(overlaps_maskless(box_at({ 0, 0 }, { 20, 20 }), box_at({ 25, 0 }, { 20, 20 })));

    // A 45-degree box reaches further along the diagonal (half diagonal ~14.14 vs half size 10).
    auto eighth_turn = glm::quarter_pi<float>();
    REQUIRE_FALSE(overlaps_maskless(box_at({ 0, 0 }, { 20, 20 }), box_at({ 25, 0 }, { 20, 20 }, eighth_turn)));
    REQUIRE(overlaps_maskless(box_at({ 0, 0 }, { 20, 20 }), box_at({ 24, 0 }, { 20, 20 }, eighth_turn)));
}

TEST_CASE("pill vs circle") {
    auto pill = pill_at({ 0, 0 }, 5, 10); // vertical: segment y in [-10, 10], radius 5
    REQUIRE(overlaps_maskless(circle_at({ 0, 18 }, 4), pill));
    REQUIRE_FALSE(overlaps_maskless(circle_at({ 0, 20 }, 4), pill));
    REQUIRE(overlaps_maskless(circle_at({ 8, 0 }, 4), pill));
    REQUIRE_FALSE(overlaps_maskless(circle_at({ 10, 0 }, 4), pill));

    auto horizontal = pill_at({ 0, 0 }, 5, 10, glm::half_pi<float>());
    REQUIRE(overlaps_maskless(circle_at({ 18, 0 }, 4), horizontal));
    REQUIRE_FALSE(overlaps_maskless(circle_at({ 0, 18 }, 4), horizontal));
}

TEST_CASE("pill vs pill") {
    REQUIRE(overlaps_maskless(pill_at({ 0, 0 }, 5, 10), pill_at({ 9, 0 }, 5, 10)));
    REQUIRE(overlaps_maskless(pill_at({ 0, 0 }, 5, 10), pill_at({ 10, 0 }, 5, 10))); // touching
    REQUIRE_FALSE(overlaps_maskless(pill_at({ 0, 0 }, 5, 10), pill_at({ 11, 0 }, 5, 10)));
    // Crossed pills overlap at the intersection point.
    REQUIRE(overlaps_maskless(pill_at({ 0, 0 }, 2, 20), pill_at({ 0, 0 }, 2, 20, glm::half_pi<float>())));
}

TEST_CASE("pill vs box") {
    auto pill = pill_at({ 0, 0 }, 5, 10);
    REQUIRE(overlaps_maskless(pill, box_at({ 10, 0 }, { 10, 40 }))); // box edge at x=5, touching
    REQUIRE_FALSE(overlaps_maskless(pill, box_at({ 12, 0 }, { 10, 40 })));
    REQUIRE(overlaps_maskless(pill_at({ 0, 0 }, 2, 30), box_at({ 0, 0 }, { 10, 10 }))); // segment crosses the box
    // Cap (not the segment body) reaching a box above.
    REQUIRE(overlaps_maskless(pill, box_at({ 0, 17 }, { 20, 4 })));
    REQUIRE_FALSE(overlaps_maskless(pill, box_at({ 0, 18.1f }, { 20, 4 })));
}

TEST_CASE("world_collider derives center and rotation from the transform") {
    TransformComponent transform {};
    transform.translation = { 100.0f, 50.0f, 3.0f };
    transform.orientation = glm::quat(glm::vec3 { 0.0f, 0.0f, glm::half_pi<float>() });

    auto world = k2::collision::world_collider(ColliderComponent { .shape = BoxShape { .size = { 40, 4 } } }, transform);
    REQUIRE(world.center.x == Catch::Approx(100.0f));
    REQUIRE(world.center.y == Catch::Approx(50.0f));

    // The 40-wide box is rotated upright: it reaches (100, 50+15) but not (100+15, 50).
    REQUIRE(overlaps_maskless(circle_at({ 100, 65 }, 5), world));
    REQUIRE_FALSE(overlaps_maskless(circle_at({ 115, 50 }, 5), world));
}

TEST_CASE("registry queries") {
    entt::registry registry;

    auto make = [&](glm::vec2 position, float radius) {
        auto entity = registry.create();
        auto& transform = registry.emplace<TransformComponent>(entity);
        transform.translation = { position, 0.0f };
        registry.emplace<ColliderComponent>(entity, ColliderComponent { .shape = CircleShape { .radius = radius } });
        return entity;
    };

    auto near = make({ 0, 0 }, 10);
    auto far = make({ 100, 0 }, 10);
    auto transform_only = registry.create();
    registry.emplace<TransformComponent>(transform_only);
    auto collider_only = registry.create();
    registry.emplace<ColliderComponent>(collider_only);

    SECTION("query_circle returns only overlapping collider entities") {
        auto hits = k2::collision::query_circle(registry, { 0, 0 }, 50);
        REQUIRE(hits == std::vector { near });
        auto all = k2::collision::query_circle(registry, { 50, 0 }, 60);
        REQUIRE(all.size() == 2);
    }

    SECTION("query_aabb uses half extents around the center") {
        auto hits = k2::collision::query_aabb(registry, { 80, 0 }, { 15, 15 });
        REQUIRE(hits == std::vector { far });
    }

    SECTION("overlaps requires collider and transform on both entities") {
        REQUIRE(k2::collision::overlaps(registry, near, near));
        REQUIRE_FALSE(k2::collision::overlaps(registry, near, far));
        REQUIRE_FALSE(k2::collision::overlaps(registry, near, transform_only));
        REQUIRE_FALSE(k2::collision::overlaps(registry, near, collider_only));
    }

    SECTION("query_point is a zero-radius circle query") {
        REQUIRE(k2::collision::query_point(registry, { 5, 5 }) == std::vector { near });
        REQUIRE(k2::collision::query_point(registry, { 0, 10 }) == std::vector { near }); // boundary
        REQUIRE(k2::collision::query_point(registry, { 50, 0 }).empty());
    }
}

TEST_CASE("layers and masks") {
    entt::registry registry;

    auto make = [&](glm::vec2 position, std::uint32_t layer, std::uint32_t mask) {
        auto entity = registry.create();
        auto& transform = registry.emplace<TransformComponent>(entity);
        transform.translation = { position, 0.0f };
        registry.emplace<ColliderComponent>(
            entity, ColliderComponent { .shape = CircleShape { .radius = 10 }, .layer = layer, .mask = mask });
        return entity;
    };

    auto enemy = make({ 0, 0 }, 1, 0xffffffff);
    auto projectile = make({ 5, 0 }, 2, 1);
    auto other_projectile = make({ 8, 0 }, 2, 1);

    auto sorted = [](std::vector<entt::entity> entities) {
        std::ranges::sort(entities);
        return entities;
    };

    SECTION("queries filter by layer against the mask") {
        REQUIRE(k2::collision::query_circle(registry, { 0, 0 }, 50, 1) == std::vector { enemy });
        REQUIRE(sorted(k2::collision::query_circle(registry, { 0, 0 }, 50, 2))
            == sorted({ projectile, other_projectile }));
        REQUIRE(k2::collision::query_circle(registry, { 0, 0 }, 50).size() == 3);
        REQUIRE(sorted(k2::collision::query_point(registry, { 5, 0 }, 2)) == sorted({ projectile, other_projectile }));
    }

    SECTION("overlaps requires each mask to include the other's layer") {
        REQUIRE(k2::collision::overlaps(registry, enemy, projectile));
        // Projectiles only interact with layer 1: two overlapping projectiles don't collide.
        REQUIRE_FALSE(k2::collision::overlaps(registry, projectile, other_projectile));
    }
}
