#pragma once

#include "core/scene.hpp"
#include "kione2D.hpp"
#include "rendering/renderer2D.hpp"
#include "serializers/core/scene.hpp"

#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <glm/glm.hpp>
#include <numeric>

using namespace k2::literals;

class SceneLayer : public k2::Layer {
    k2::Window& window;

    entt::registry registry {};
    k2::Renderer2D renderer2D;

public:
    explicit SceneLayer(k2::Window& window)
        : window { window } {
        k2::Resources::get<k2::Texture2D>()["white"_fnv1a] = k2::Texture2D::create_white_texture();
        k2::Resources::get<k2::Texture2D>()["tex"_fnv1a] = k2::Texture2D { k2::Image("res/textures/texture.jpg") };

        setup_scene();
    }

    SceneLayer(const SceneLayer&) = delete;

    SceneLayer& operator=(const SceneLayer&) = delete;

    void setup_scene() {
        auto entity = registry.create();

        auto& sprite = registry.emplace<k2::SpriteComponent>(entity);
        sprite = k2::SpriteComponent {
            .color = { 1.0f, 0.0f, 0.0f, 1.0f },
            .texture = "tex"_fnv1a,
        };

        auto& transform = registry.emplace<k2::TransformComponent>(entity);
        transform = k2::TransformComponent {
            .scale { 300.0f, 300.0f, 1.0f },
        };

        auto& camera = registry.emplace<k2::Camera>(entity);
        camera = k2::Camera {
            .position { 0, 0, 1000.f },
            .target { 0, 0, 0 },
            .up { 0, 1.0f, 0 },

            .projection_traits { k2::Camera::OrthographicTraits {
                .left = -float(window.get_width()),
                .right = float(window.get_width()),
                .top = float(window.get_height()),
                .bottom = -float(window.get_height()),
                .far_clip = -1000.f,
                .near_clip = 1000.f,
            } },
        };
    }

    void update(float) override { }

    void render() override {
        registry.view<k2::Camera>().each([&](auto, const auto& camera) { renderer2D.camera = camera; });
        registry.view<k2::TransformComponent, k2::SpriteComponent>().each(
            [&](auto, const auto& transform, const auto& sprite) { renderer2D.draw(transform, sprite); });
        renderer2D.render();
    }

    bool handle_event(const k2::Event*) override { return false; }
};
