#pragma once

#include "kione2D.hpp"

#include "core/rendering/camera.hpp"
#include "core/rendering/renderer2D.hpp"
#include "core/rendering/shader.hpp"

#include "core/entity_editor.hpp"
#include "core/logger.hpp"

#include "components.hpp"

#include "glm/glm.hpp"

#include <numeric>

using namespace k2::literals;

std::vector<k2::Renderer2D::Vertex> vertices {
    k2::Renderer2D::Vertex {
        .position = { 1.0f, -1.0f, 0.0f },
        .color = { 1.0f, 1.0f, 1.0f, 1.0f },
        .texture_coordinate = { 1.0f, 0.0f },
        .texture = "tex"_fnv1a,
    },
    k2::Renderer2D::Vertex {
        .position = { -1.0f, 1.0f, 0.0f },
        .color = { 1.0f, 1.0f, 1.0f, 1.0f },
        .texture_coordinate = { 0.0f, 1.0f },
        .texture = "tex"_fnv1a,
    },
    k2::Renderer2D::Vertex {
        .position = { -1.0f, -1.0f, 0.0f },
        .color = { 1.0f, 1.0f, 1.0f, 1.0f },
        .texture_coordinate = { 0.0f, 0.0f },
        .texture = "tex"_fnv1a,
    },
    k2::Renderer2D::Vertex {
        .position = { 1.0f, 1.0f, 0.0f },
        .color = { 1.0f, 1.0f, 1.0f, 1.0f },
        .texture_coordinate = { 1.0f, 1.0f },
        .texture = "tex"_fnv1a,
    },
};

std::vector<std::uint32_t> indices { 0, 1, 2, 0, 3, 1 };

class SceneLayer : public k2::Layer {
    k2::Window& window;

    entt::registry registry {};
    k2::EntityEditor<entt::registry::entity_type> entity_editor {};

    k2::Program program {};
    k2::Renderer2D renderer2D;

public:
    explicit SceneLayer(k2::Window& window)
        : window { window } {
        auto program_loader = [](auto vertex, auto fragment) {
            namespace fs = std::filesystem;
            auto vertex_shader = k2::Shader(GL_VERTEX_SHADER, fs::path(vertex));
            if (!vertex_shader) {
                k2::Logger::app->critical(vertex_shader.error_msg().value());
            }

            auto fragment_shader = k2::Shader(GL_FRAGMENT_SHADER, fs::path(fragment));
            if (!fragment_shader) {
                k2::Logger::app->critical(fragment_shader.error_msg().value());
            }

            k2::Program ret { std::move(vertex_shader), std::move(fragment_shader) };
            ret.link();

            if (!ret) {
                k2::Logger::app->critical(ret.error_msg().value());
            }
            return ret;
        };
        program = program_loader("res/shaders/2d_vs.glsl", "res/shaders/2d_fs.glsl");

        k2::Resources::get<k2::Texture2D>()["white"_fnv1a] = k2::Texture2D::create_white_texture();
        k2::Resources::get<k2::Texture2D>()["tex"_fnv1a] = k2::Texture2D { "res/textures/texture.jpg" };

        entity_editor.register_component<Transform>("Transform");
        setup_scene();
    }

    SceneLayer(const SceneLayer&) = delete;

    SceneLayer& operator=(const SceneLayer&) = delete;

    void setup_scene() {
        renderer2D.camera = k2::Camera {
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
        auto transform = glm::scale(glm::mat4(1.0f), glm::vec3(300.f, 300.f, 1.f));
        renderer2D.draw(vertices, indices, transform);
        renderer2D.render();
    }

    bool handle_event(const k2::Event*) override { return false; }
};
