#pragma once

#include "core/animation_system.hpp"
#include "core/project.hpp"
#include "core/runtime.hpp"
#include "core/scene.hpp"
#include "core/scene_loader.hpp"
#include "kione2D.hpp"
#include "rendering/renderer2D.hpp"

#include <format>
#include <glm/glm.hpp>
#include <stdexcept>

#include "core/logger.hpp"

template <class T> T value_or_abort(std::expected<T, std::string> result) {
    if (!result) {
        throw std::runtime_error(result.error());
    }
    return std::move(*result);
}

class GameLayer : public k2::Layer {
    k2::Window& window;
    k2::AssetRegistry assets;
    k2::Runtime runtime { window };
    k2::Scene scene;
    k2::Renderer2D renderer2D {};

public:
    explicit GameLayer(k2::Window& window, const std::string& project_path)
        : window { window } {
        auto project = value_or_abort(k2::Project::load(project_path));
        assets = std::move(project.assets);
        scene = value_or_abort(k2::SceneLoader::load(project.main_scene, runtime.resources, assets));
        attach_scene_context();
        publish_scene_view();
    }

    GameLayer(const GameLayer&) = delete;

    GameLayer& operator=(const GameLayer&) = delete;

    void fixed_update(float dt) override { runtime.scripts.fixed_update(scene, assets, dt); }

    void update(float dt) override {
        runtime.scripts.update(scene, assets, dt);
        k2::AnimationSystem::update(scene, dt);
        runtime.audio.update(scene);
        apply_scene_request();
        publish_scene_view();
    }

    void attach_scene_context() {
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
        scene.registry.ctx().emplace<k2::Runtime&>(runtime);
    }

    void apply_scene_request() {
        const auto* request = scene.registry.ctx().find<k2::SceneRequest>();
        if (request == nullptr) {
            return;
        }
        auto loaded = k2::SceneLoader::load(request->scene, runtime.resources, assets);
        if (!loaded) {
            k2::Log::app().error(std::format("Scene switch failed: {}", loaded.error()));
            scene.registry.ctx().erase<k2::SceneRequest>();
            return;
        }
        scene = std::move(*loaded);
        attach_scene_context();
    }

    void publish_scene_view() {
        const auto* main_camera = k2::find_main_camera(scene.registry);
        const auto& design = main_camera ? *main_camera : scene.registry.ctx().get<k2::Camera>();
        scene.registry.ctx().insert_or_assign(
            design.for_surface(float(window.get_width()), float(window.get_height())));
    }

    void render() override {
        renderer2D.camera = scene.registry.ctx().get<k2::SceneView>().camera;
        renderer2D.draw(scene);
        renderer2D.render();
    }

    bool handle_event(const k2::Event* event) override {
        return runtime.scripts.handle_event(scene, assets, event);
    }
};
