#pragma once

#include "core/project.hpp"
#include "core/scene.hpp"
#include "core/scene_loader.hpp"
#include "kione2D.hpp"
#include "rendering/renderer2D.hpp"

#include <glm/glm.hpp>

class SceneLayer : public k2::Layer {
    k2::Window& window;

    k2::Project project = k2::Project::load("res/project.k2project");
    k2::ResourceManager resources {};
    k2::Scene scene = k2::SceneLoader::load(project.main_scene, resources, project.assets);
    k2::Renderer2D renderer2D {};

public:
    explicit SceneLayer(k2::Window& window)
        : window { window } {
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
    }

    SceneLayer(const SceneLayer&) = delete;

    SceneLayer& operator=(const SceneLayer&) = delete;

    void update(float) override { }

    void render() override {
        renderer2D.camera = scene.registry.ctx().get<k2::Camera>();
        renderer2D.draw(scene);
        renderer2D.render();
    }

    bool handle_event(const k2::Event*) override { return false; }
};
