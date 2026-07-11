#pragma once

#include "core/scene.hpp"
#include "kione2D.hpp"
#include "rendering/renderer2D.hpp"

#include <glm/glm.hpp>

using namespace k2::literals;

class SceneLayer : public k2::Layer {
    k2::Window& window;

    k2::Scene scene {};
    k2::ResourceManager resources {};
    k2::Renderer2D renderer2D {};

public:
    explicit SceneLayer(k2::Window& window)
        : window { window } {
        scene.registry.ctx().emplace<k2::ResourceManager&>(resources);
        resources.set("white", k2::Texture2D::create_white_texture<uint8_t>());
        resources.set("tex", k2::Texture2D { k2::Image("res/textures/texture.jpg") });

        scene.registry.ctx().emplace<k2::Camera>(k2::Camera {
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
        });
        setup_scene();
    }

    SceneLayer(const SceneLayer&) = delete;

    SceneLayer& operator=(const SceneLayer&) = delete;

    void setup_scene() {
        auto entity = scene.registry.create();

        auto& sprite = scene.registry.emplace<k2::SpriteComponent>(entity);
        sprite = k2::SpriteComponent {
            .color = { 1.0f, 0.0f, 0.0f, 1.0f },
            .texture = "tex"_fnv1a,
        };

        auto& transform = scene.registry.emplace<k2::TransformComponent>(entity);
        transform = k2::TransformComponent {
            .translation {},
            .orientation {},
            .scale { 300.0f, 300.0f, 1.0f },
        };
    }

    void update(float) override { }

    void render() override {
        // draw(Scene) leaves the camera untouched when the scene has no Camera
        // component, and picks the ResourceManager out of the registry context.
        renderer2D.camera = scene.registry.ctx().get<k2::Camera>();
        renderer2D.draw(scene);
        renderer2D.render();
    }

    bool handle_event(const k2::Event*) override { return false; }
};
