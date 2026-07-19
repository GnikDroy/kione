#pragma once

#include "core/animation_system.hpp"
#include "core/project.hpp"
#include "core/scene.hpp"
#include "core/scene_loader.hpp"
#include "core/script_system.hpp"
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

class SceneLayer : public k2::Layer {
    k2::Window& window;

    k2::Project project;
    k2::ResourceManager resources {};
    // Voices reference clip PCM owned by resources, so audio dies first.
    k2::AudioSystem audio {};
    // Declared before the scenes: registries can hold LuaComponents referencing the
    // script system's lua state, so they must be destroyed first.
    k2::ScriptSystem scripts { window };
    k2::Scene scene = value_or_abort(k2::SceneLoader::load(project.main_scene, resources, project.assets));
    k2::Renderer2D renderer2D {};

public:
    explicit SceneLayer(k2::Window& window, const std::string& project_path)
        : window { window }
        , project { value_or_abort(k2::Project::load(project_path)) } {
        attach_scene_context();
        publish_scene_view();
    }

    SceneLayer(const SceneLayer&) = delete;

    SceneLayer& operator=(const SceneLayer&) = delete;

    void fixed_update(float dt) override { scripts.fixed_update(scene, project.assets, dt); }

    void update(float dt) override {
        scripts.update(scene, project.assets, dt);
        k2::AnimationSystem::update(scene, dt);
        audio.update(scene);
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
        scene.registry.ctx().emplace<k2::AudioSystem&>(audio);
    }

    void apply_scene_request() {
        const auto* request = scene.registry.ctx().find<k2::SceneRequest>();
        if (request == nullptr) {
            return;
        }
        auto loaded = k2::SceneLoader::load(request->scene, resources, project.assets);
        if (!loaded) {
            k2::Log::app().error(std::format("Scene switch failed: {}", loaded.error()));
            scene.registry.ctx().erase<k2::SceneRequest>();
            return;
        }
        scene = std::move(*loaded);

        // Reset explicitly!
        // The registry still lives at the same address. Checks in ScriptSystem/AudioSystem cannot see it.
        scripts.clear_cache();
        audio.stop_all();
        attach_scene_context();
    }

    void publish_scene_view() {
        const auto* main_camera = k2::find_main_camera(scene.registry);
        scene.registry.ctx().insert_or_assign(k2::SceneView {
            .camera = main_camera ? *main_camera : scene.registry.ctx().get<k2::Camera>(),
            .viewport = { .x = 0.0f, .y = 0.0f, .w = float(window.get_width()), .h = float(window.get_height()) },
        });
    }

    void render() override {
        renderer2D.camera = scene.registry.ctx().get<k2::SceneView>().camera;
        renderer2D.draw(scene);
        renderer2D.render();
    }

    bool handle_event(const k2::Event* event) override { return scripts.handle_event(scene, project.assets, event); }
};
