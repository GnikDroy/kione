#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "components/animation.hpp"
#include "components/sprite.hpp"
#include "core/animation_system.hpp"
#include "core/resources.hpp"
#include "core/scene.hpp"
#include "serializers/asset/sprite_animation.hpp"
#include "serializers/components/animation.hpp"

using Catch::Approx;
using namespace k2::literals;

namespace {
k2::SpriteAnimation traffic_light() {
    return k2::SpriteAnimation {
        .texture = k2::AssetHandle { "sheet" },
        .frames {
            { .uv { 0.0f, 0.0f, 0.5f, 1.0f }, .duration = 0.4f, .color { 1.0f, 0.0f, 0.0f, 1.0f } },
            { .uv { 0.5f, 0.0f, 0.5f, 1.0f }, .duration = 0.1f, .color { 0.0f, 1.0f, 0.0f, 1.0f } },
        },
    };
}
}

TEST_CASE("length and frame_at follow per-frame durations", "[animation]") {
    auto clip = traffic_light();
    REQUIRE(clip.length() == Approx(0.5f));

    REQUIRE(clip.frame_at(0.0f).color.x == Approx(1.0f));
    REQUIRE(clip.frame_at(0.39f).color.x == Approx(1.0f));
    REQUIRE(clip.frame_at(0.41f).color.y == Approx(1.0f));
    // Past the end resolves to the last frame.
    REQUIRE(clip.frame_at(9.0f).color.y == Approx(1.0f));
}

TEST_CASE("AnimationSystem applies uv and color and loops over uneven frames", "[animation]") {
    k2::Scene scene;
    k2::ResourceManager resources;
    scene.registry.ctx().emplace<k2::ResourceManager&>(resources);
    resources.set("lights", traffic_light());

    auto entity = scene.registry.create();
    scene.registry.emplace<k2::SpriteComponent>(entity);
    scene.registry.emplace<k2::AnimationComponent>(entity).clip = k2::AssetHandle { "lights" };

    k2::AnimationSystem::update(scene, 0.0f);
    auto& sprite = scene.registry.get<k2::SpriteComponent>(entity);
    REQUIRE(sprite.texture.name == "sheet");
    REQUIRE(sprite.uv_rect.x == Approx(0.0f));
    REQUIRE(sprite.color.x == Approx(1.0f));

    // 0.45s: inside the short second frame.
    k2::AnimationSystem::update(scene, 0.45f);
    REQUIRE(sprite.uv_rect.x == Approx(0.5f));
    REQUIRE(sprite.color.y == Approx(1.0f));

    // 0.65s total wraps to 0.15s: first frame again.
    k2::AnimationSystem::update(scene, 0.2f);
    REQUIRE(sprite.uv_rect.x == Approx(0.0f));
    REQUIRE(sprite.color.x == Approx(1.0f));
}

TEST_CASE("AnimationSystem clamps and stops non-looping clips", "[animation]") {
    k2::Scene scene;
    k2::ResourceManager resources;
    scene.registry.ctx().emplace<k2::ResourceManager&>(resources);
    auto clip = traffic_light();
    clip.loop = false;
    resources.set("once", std::move(clip));

    auto entity = scene.registry.create();
    scene.registry.emplace<k2::SpriteComponent>(entity);
    auto& animation = scene.registry.emplace<k2::AnimationComponent>(entity);
    animation.clip = k2::AssetHandle { "once" };

    REQUIRE_FALSE(animation.finished);
    k2::AnimationSystem::update(scene, 1.0f);
    auto& sprite = scene.registry.get<k2::SpriteComponent>(entity);
    REQUIRE(sprite.uv_rect.x == Approx(0.5f));
    REQUIRE_FALSE(animation.playing);
    REQUIRE(animation.finished);

    // Stopped: elapsed no longer advances.
    auto elapsed = animation.elapsed;
    k2::AnimationSystem::update(scene, 1.0f);
    REQUIRE(animation.elapsed == Approx(elapsed));
}

TEST_CASE("AnimationSystem respects speed and paused state", "[animation]") {
    k2::Scene scene;
    k2::ResourceManager resources;
    scene.registry.ctx().emplace<k2::ResourceManager&>(resources);
    resources.set("lights", traffic_light());

    auto entity = scene.registry.create();
    scene.registry.emplace<k2::SpriteComponent>(entity);
    auto& animation = scene.registry.emplace<k2::AnimationComponent>(entity);
    animation.clip = k2::AssetHandle { "lights" };
    animation.speed = 2.0f;

    // 0.22s at 2x -> 0.44s: second frame.
    k2::AnimationSystem::update(scene, 0.22f);
    auto& sprite = scene.registry.get<k2::SpriteComponent>(entity);
    REQUIRE(sprite.uv_rect.x == Approx(0.5f));

    animation.playing = false;
    k2::AnimationSystem::update(scene, 10.0f);
    REQUIRE(sprite.uv_rect.x == Approx(0.5f));
}

TEST_CASE("SpriteAnimation serializer round-trips", "[animation]") {
    auto clip = traffic_light();
    clip.loop = false;

    auto loaded = YAML::Node { clip }.as<k2::SpriteAnimation>();
    REQUIRE(loaded.texture.name == "sheet");
    REQUIRE(loaded.texture.id == "sheet"_fnv1a);
    REQUIRE(loaded.loop == false);
    REQUIRE(loaded.frames.size() == 2);
    REQUIRE(loaded.frames[0].uv.w == Approx(0.5f));
    REQUIRE(loaded.frames[0].duration == Approx(0.4f));
    REQUIRE(loaded.frames[0].color.x == Approx(1.0f));
    REQUIRE(loaded.frames[1].duration == Approx(0.1f));
    REQUIRE(loaded.frames[1].color.y == Approx(1.0f));
}

TEST_CASE("SpriteAnimation frame defaults apply when omitted", "[animation]") {
    auto node = YAML::Load("texture: sheet\nframes:\n  - uv: [0, 0, 1, 1]\n");
    auto clip = node.as<k2::SpriteAnimation>();
    REQUIRE(clip.loop == true);
    REQUIRE(clip.frames.size() == 1);
    REQUIRE(clip.frames[0].duration == Approx(0.1f));
    REQUIRE(clip.frames[0].color.x == Approx(1.0f));
    REQUIRE(clip.frames[0].color.w == Approx(1.0f));
}

TEST_CASE("AnimationComponent serializer round-trips", "[animation]") {
    k2::AnimationComponent animation {
        .clip = k2::AssetHandle { "walk" },
        .speed = 1.5f,
        .playing = false,
    };

    auto loaded = YAML::Node { animation }.as<k2::AnimationComponent>();
    REQUIRE(loaded.clip.name == "walk");
    REQUIRE(loaded.speed == Approx(1.5f));
    REQUIRE(loaded.playing == false);
    REQUIRE(loaded.elapsed == Approx(0.0f));
}
