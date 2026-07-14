#pragma once

#include "core/animation_system.hpp"
#include "core/project.hpp"
#include "core/scene.hpp"
#include "core/scene_loader.hpp"
#include "core/script_system.hpp"
#include "kione2D.hpp"
#include "rendering/renderer2D.hpp"

#include <glm/glm.hpp>
#include <stdexcept>

template <class T> T value_or_abort(std::expected<T, std::string> result) {
    if (!result) {
        throw std::runtime_error(result.error());
    }
    return std::move(*result);
}

class SceneLayer : public k2::Layer {
    k2::Window& window;

    k2::Project project = value_or_abort(k2::Project::load("res/project.k2project"));
    k2::ResourceManager resources {};
    k2::Scene scene = value_or_abort(k2::SceneLoader::load(project.main_scene, resources, project.assets));
    k2::Renderer2D renderer2D {};
    k2::ScriptSystem scripts { window };

public:
    explicit SceneLayer(k2::Window& window)
        : window { window } {
        scene.registry.ctx().emplace<k2::Camera>(k2::Camera {
            .position { 0, 0, 1000.f },
            .target { 0, 0, 0 },
            .up { 0, 1.0f, 0 },

            .projection_traits { k2::Camera::OrthographicTraits {
                .left = -float(window.get_width()) / 2.0f,
                .right = float(window.get_width()) / 2.0f,
                .top = float(window.get_height()) / 2.0f,
                .bottom = -float(window.get_height()) / 2.0f,
                .far_clip = 0.f,
                .near_clip = 2000.f,
            } },
        });
    }

    SceneLayer(const SceneLayer&) = delete;

    SceneLayer& operator=(const SceneLayer&) = delete;

    void update(float dt) override {
        scripts.update(scene, project.assets, dt);
        k2::AnimationSystem::update(scene, dt);
    }

    void render() override {
        const auto* main_camera = k2::find_main_camera(scene.registry);
        renderer2D.camera = main_camera ? *main_camera : scene.registry.ctx().get<k2::Camera>();
        renderer2D.draw(scene);
        renderer2D.render();
    }

    bool handle_event(const k2::Event*) override { return false; }
};
