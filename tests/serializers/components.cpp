#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "serializers/components/camera.hpp"
#include "serializers/components/collider.hpp"
#include "serializers/components/environment.hpp"
#include "serializers/components/light.hpp"
#include "serializers/components/relation.hpp"
#include "serializers/components/script.hpp"
#include "serializers/components/sprite.hpp"
#include "serializers/components/tag.hpp"
#include "serializers/components/transform.hpp"

using Catch::Approx;

namespace {
template <class Component> Component round_trip(const Component& component) {
    return YAML::Node { component }.as<Component>();
}
}

TEST_CASE("TransformComponent serializer round-trips", "[serializers]") {
    k2::TransformComponent transform {
        .translation { 1.0f, -2.0f, 3.0f },
        .orientation = glm::angleAxis(0.5f, glm::vec3 { 0.0f, 0.0f, 1.0f }),
        .scale { 2.0f, 3.0f, 4.0f },
    };

    auto loaded = round_trip(transform);
    REQUIRE(loaded.translation == transform.translation);
    REQUIRE(loaded.scale == transform.scale);
    REQUIRE(loaded.orientation.w == Approx(transform.orientation.w));
    REQUIRE(loaded.orientation.z == Approx(transform.orientation.z));
}

TEST_CASE("SpriteComponent serializer round-trips", "[serializers]") {
    k2::SpriteComponent sprite {
        .color { 0.1f, 0.2f, 0.3f, 0.4f },
        .texture = k2::AssetHandle { "cobble" },
        .uv_rect { 0.0f, 0.5f, 4.0f, 2.0f },
    };

    auto loaded = round_trip(sprite);
    REQUIRE(loaded.color == sprite.color);
    REQUIRE(loaded.texture.name == "cobble");
    REQUIRE(loaded.texture.id == sprite.texture.id);
    REQUIRE(loaded.uv_rect.x == sprite.uv_rect.x);
    REQUIRE(loaded.uv_rect.y == sprite.uv_rect.y);
    REQUIRE(loaded.uv_rect.w == sprite.uv_rect.w);
    REQUIRE(loaded.uv_rect.h == sprite.uv_rect.h);
}

TEST_CASE("ColliderComponent serializer round-trips every shape", "[serializers]") {
    auto box = round_trip(k2::ColliderComponent { .shape = k2::BoxShape { .size = { 12.0f, 34.0f } } });
    REQUIRE(std::get<k2::BoxShape>(box.shape).size == glm::vec2 { 12.0f, 34.0f });

    auto circle = round_trip(k2::ColliderComponent { .shape = k2::CircleShape { .radius = 7.5f } });
    REQUIRE(std::get<k2::CircleShape>(circle.shape).radius == 7.5f);

    auto pill = round_trip(k2::ColliderComponent { .shape = k2::PillShape { .radius = 5.0f, .half_height = 9.0f } });
    REQUIRE(std::get<k2::PillShape>(pill.shape).radius == 5.0f);
    REQUIRE(std::get<k2::PillShape>(pill.shape).half_height == 9.0f);
    REQUIRE(pill.layer == 1);
    REQUIRE(pill.mask == 0xffffffff);

    auto filtered = round_trip(
        k2::ColliderComponent { .shape = k2::CircleShape { .radius = 1.0f }, .layer = 2, .mask = 5 });
    REQUIRE(filtered.layer == 2);
    REQUIRE(filtered.mask == 5);
}

TEST_CASE("ColliderComponent serializer rejects unknown shapes", "[serializers]") {
    auto node = YAML::Load("{Shape: Sphere, Radius: 5}");
    k2::ColliderComponent collider;
    REQUIRE_FALSE(YAML::convert<k2::ColliderComponent>::decode(node, collider));
}

TEST_CASE("Environment serializer round-trips", "[serializers]") {
    auto loaded = round_trip(k2::Environment { .bloom = false, .bloom_intensity = 0.5f, .bloom_threshold = 1.5f });
    REQUIRE(loaded.bloom == false);
    REQUIRE(loaded.bloom_intensity == 0.5f);
    REQUIRE(loaded.bloom_threshold == 1.5f);

    // Missing fields decode to defaults.
    auto defaults = YAML::Load("{}").as<k2::Environment>();
    REQUIRE(defaults.bloom);
    REQUIRE(defaults.bloom_intensity == 1.0f);
    REQUIRE(defaults.bloom_threshold == 1.0f);
}

TEST_CASE("TagComponent serializer round-trips as a scalar", "[serializers]") {
    k2::TagComponent tag { "Player One" };
    auto loaded = round_trip(tag);
    REQUIRE(loaded.tag == "Player One");
    REQUIRE_THROWS(YAML::Load("[1, 2]").as<k2::TagComponent>());
}

TEST_CASE("ScriptComponent serializer round-trips", "[serializers]") {
    k2::ScriptComponent script { .script = k2::AssetHandle { "mover" } };
    auto loaded = round_trip(script);
    REQUIRE(loaded.script.name == "mover");
    REQUIRE(loaded.script.id == script.script.id);
}

TEST_CASE("RelationComponent serializer round-trips", "[serializers]") {
    k2::RelationComponent relation {
        .children = 2,
        .parent = entt::entity { 7 },
        .first = entt::entity { 9 },
        .prev = entt::entity { 11 },
        .next = entt::entity { 13 },
    };

    auto loaded = round_trip(relation);
    REQUIRE(loaded.children == 2);
    REQUIRE(loaded.parent == entt::entity { 7 });
    REQUIRE(loaded.first == entt::entity { 9 });
    REQUIRE(loaded.prev == entt::entity { 11 });
    REQUIRE(loaded.next == entt::entity { 13 });
}

TEST_CASE("RelationComponent serializer preserves null entities", "[serializers]") {
    auto loaded = round_trip(k2::RelationComponent {});
    REQUIRE((loaded.parent == entt::null));
    REQUIRE((loaded.first == entt::null));
}

TEST_CASE("Camera serializer round-trips orthographic traits", "[serializers]") {
    k2::Camera camera {
        .position { 1.0f, 2.0f, 3.0f },
        .target { 4.0f, 5.0f, 6.0f },
        .up { 0.0f, 1.0f, 0.0f },
        .projection_traits { k2::Camera::OrthographicTraits {
            .left = -100.0f,
            .right = 100.0f,
            .top = 50.0f,
            .bottom = -50.0f,
            .far_clip = 0.0f,
            .near_clip = 2000.0f,
        } },
    };

    auto loaded = round_trip(camera);
    REQUIRE(loaded.position == camera.position);
    REQUIRE(loaded.target == camera.target);
    auto* traits = std::get_if<k2::Camera::OrthographicTraits>(&loaded.projection_traits);
    REQUIRE(traits != nullptr);
    REQUIRE(traits->left == -100.0f);
    REQUIRE(traits->right == 100.0f);
    REQUIRE(traits->top == 50.0f);
    REQUIRE(traits->bottom == -50.0f);
    REQUIRE(traits->far_clip == 0.0f);
    REQUIRE(traits->near_clip == 2000.0f);
}

TEST_CASE("Camera serializer round-trips perspective traits", "[serializers]") {
    k2::Camera camera {
        .projection_traits { k2::Camera::PerspectiveTraits {
            .fov = 1.2f,
            .aspect_ratio = 1.5f,
            .far_clip = 1000.0f,
            .near_clip = 0.1f,
        } },
    };

    auto loaded = round_trip(camera);
    auto* traits = std::get_if<k2::Camera::PerspectiveTraits>(&loaded.projection_traits);
    REQUIRE(traits != nullptr);
    REQUIRE(traits->fov == Approx(1.2f));
    REQUIRE(traits->aspect_ratio == Approx(1.5f));
}

TEST_CASE("AmbientLight serializer round-trips", "[serializers]") {
    auto loaded = round_trip(k2::AmbientLight { .color { 0.4f, 0.4f, 0.6f }, .intensity = 0.35f });
    REQUIRE(loaded.color == glm::vec3 { 0.4f, 0.4f, 0.6f });
    REQUIRE(loaded.intensity == Approx(0.35f));
}

TEST_CASE("PointLight serializer round-trips", "[serializers]") {
    auto loaded = round_trip(k2::PointLight { .color { 1.0f, 0.8f, 0.5f }, .intensity = 2.0f, .radius = 700.0f });
    REQUIRE(loaded.color == glm::vec3 { 1.0f, 0.8f, 0.5f });
    REQUIRE(loaded.intensity == Approx(2.0f));
    REQUIRE(loaded.radius == Approx(700.0f));
}

TEST_CASE("SpotLight serializer round-trips", "[serializers]") {
    auto loaded = round_trip(k2::SpotLight {
        .color { 0.0f, 1.0f, 0.0f },
        .intensity = 3.0f,
        .radius = 400.0f,
        .inner_angle = 0.2f,
        .outer_angle = 0.5f,
    });
    REQUIRE(loaded.color == glm::vec3 { 0.0f, 1.0f, 0.0f });
    REQUIRE(loaded.intensity == Approx(3.0f));
    REQUIRE(loaded.radius == Approx(400.0f));
    REQUIRE(loaded.inner_angle == Approx(0.2f));
    REQUIRE(loaded.outer_angle == Approx(0.5f));
}

TEST_CASE("SpriteLight serializer round-trips", "[serializers]") {
    auto loaded = round_trip(k2::SpriteLight {
        .texture = k2::AssetHandle { "glow" },
        .color { 1.0f, 0.0f, 1.0f },
        .intensity = 1.5f,
    });
    REQUIRE(loaded.texture.name == "glow");
    REQUIRE(loaded.color == glm::vec3 { 1.0f, 0.0f, 1.0f });
    REQUIRE(loaded.intensity == Approx(1.5f));
}

TEST_CASE("Map-shaped component serializers reject scalars", "[serializers]") {
    auto scalar = YAML::Load("42");
    REQUIRE_THROWS(scalar.as<k2::TransformComponent>());
    REQUIRE_THROWS(scalar.as<k2::SpriteComponent>());
    REQUIRE_THROWS(scalar.as<k2::RelationComponent>());
    REQUIRE_THROWS(scalar.as<k2::Camera>());
    REQUIRE_THROWS(scalar.as<k2::PointLight>());
}
