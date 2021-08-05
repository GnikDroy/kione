#pragma once

#include "components.hpp"
#include "kione2D.hpp"
#include "rendering/model.hpp"
#include "skybox.hpp"

class SceneLayer : public k2::Layer {
    k2::Window& window;

    entt::registry registry {};

    k2::Program light_program {};
    k2::Program skybox_program {};

    FPCamera fp_camera {};

    SkyBox skybox {};

    struct MouseController {
        float pitch = 0.0f;
        float yaw = -90.0f;
        float sensitivity = 0.1f;

        float last_cursor_x {};
        float last_cursor_y {};
    };

    MouseController mouse_controller {};

public:
    explicit SceneLayer(k2::Window& window)
        : window { window }
        , mouse_controller { .last_cursor_x = float(window.get_width()) / 2,
            .last_cursor_y = float(window.get_height()) / 2 } {
        auto program_loader = [](auto vertex, auto fragment) {
            namespace fs = std::filesystem;
            auto vertex_shader = k2::Shader(GL_VERTEX_SHADER, fs::path(vertex));
            if (!vertex_shader) {
                k2::Log::app().critical(vertex_shader.error_msg().value());
            }

            auto fragment_shader = k2::Shader(GL_FRAGMENT_SHADER, fs::path(fragment));
            if (!fragment_shader) {
                k2::Log::app().critical(fragment_shader.error_msg().value());
            }

            k2::Program ret { std::move(vertex_shader), std::move(fragment_shader) };
            ret.link();

            if (!ret) {
                k2::Log::app().critical(ret.error_msg().value());
            }
            return ret;
        };
        light_program = program_loader("res/shaders/phong_vs.glsl", "res/shaders/phong_fs.glsl");
        skybox_program = program_loader("res/shaders/skybox_vs.glsl", "res/shaders/skybox_fs.glsl");

        setup_scene();
    }

    SceneLayer(const SceneLayer&) = delete;

    SceneLayer& operator=(const SceneLayer&) = delete;

    void setup_scene() {
        fp_camera = {
            .camera { .position {},
                .target { 0.f, 0.f, -1.f },
                .up { 0.f, 1.f, 0.f },

                .projection_traits {
                    k2::Camera::PerspectiveTraits {
                        .fov = glm::radians(45.0f),
                        .aspect_ratio = float(window.get_width()) / float(window.get_height()),
                        .far_clip = 1000.f,
                        .near_clip = 0.01f,
                    },
                } },
            .direction { 0.f, 0.f, -1.f },
        };

        auto backpack = registry.create();
        registry.emplace<Transform>(backpack);
        registry.emplace<k2::Model>(backpack, "res/models/backpack.obj");

        auto light = registry.create();
        registry.emplace<Transform>(light,
            Transform {
                .position { 0.0f, 1.0f, -2.0f },
            });
        registry.emplace<PointLight>(light,
            PointLight {
                .ambient { 0.2f, 0.2f, 0.2f }, .diffuse { 0.5f, 0.5f, 0.5f }, .specular { 1.0f, 1.0f, 1.0f } });

        skybox.load({ "res/textures/skybox/right.jpg", "res/textures/skybox/left.jpg", "res/textures/skybox/bottom.jpg",
            "res/textures/skybox/top.jpg", "res/textures/skybox/front.jpg", "res/textures/skybox/back.jpg" });
    }

    void update(float dt) override {
        float camera_speed = 1.5f;

        using k2::KeyboardDevice;

        if (window.keyboard.get_state(KeyboardDevice::KeyCode::key_w) == KeyboardDevice::KeyState::press) {
            fp_camera.camera.position += camera_speed * dt * fp_camera.direction;
        } else if (window.keyboard.get_state(KeyboardDevice::KeyCode::key_s) == KeyboardDevice::KeyState::press) {
            fp_camera.camera.position -= camera_speed * dt * fp_camera.direction;
        } else if (window.keyboard.get_state(KeyboardDevice::KeyCode::key_a) == KeyboardDevice::KeyState::press) {
            fp_camera.camera.position
                -= glm::normalize(glm::cross(fp_camera.direction, fp_camera.camera.up)) * camera_speed * dt;
        } else if (window.keyboard.get_state(KeyboardDevice::KeyCode::key_d) == KeyboardDevice::KeyState::press) {
            fp_camera.camera.position
                += glm::normalize(glm::cross(fp_camera.direction, fp_camera.camera.up)) * dt * camera_speed;
        }
    }

    void render() override {
        fp_camera.update();
        auto view_mat = fp_camera.camera.get_view();
        auto projection_mat = fp_camera.camera.get_projection();

        skybox_program.use()
            .set_uniform("view", glm::mat4(glm::mat3(view_mat)))
            .set_uniform("projection", projection_mat);
        skybox.draw(skybox_program);

        registry.view<Transform, k2::Model>().each([&](auto, auto& transform, auto& model) {
            auto model_mat = [&]() {
                auto translate = glm::translate(glm::mat4(1.0f), transform.position);
                auto rotate = glm::eulerAngleXYZ(transform.rotation.x, transform.rotation.y, transform.rotation.z);
                return glm::scale(translate * rotate, transform.scale);
            }();

            registry.view<Transform, PointLight>().each([&](auto, auto& light_transform, auto& light) {
                light_program.use()
                    .set_uniform("model", model_mat)
                    .set_uniform("view", view_mat)
                    .set_uniform("projection", projection_mat)
                    .set_uniform("light.position", light_transform.position)
                    .set_uniform("light.ambient", light.ambient)
                    .set_uniform("light.diffuse", light.diffuse)
                    .set_uniform("light.specular", light.specular)
                    .set_uniform("viewer_position", fp_camera.camera.position)
                    .set_uniform("material.shininess", 32.0f);
                model.draw(light_program);
            });
        });
    }

    bool handle_event(const k2::Event* event) override {
        using namespace k2::literals;

        if (event->type == "CursorPositionEvent"_fnv1a) {
            auto e = reinterpret_cast<const k2::CursorPositionEvent*>(event);
            auto diffX = -float(mouse_controller.last_cursor_x - e->x);
            auto diffY = float(mouse_controller.last_cursor_y - e->y);

            mouse_controller.yaw += diffX * mouse_controller.sensitivity;
            mouse_controller.pitch += diffY * mouse_controller.sensitivity;

            if (mouse_controller.pitch > 89.0f)
                mouse_controller.pitch = 89.0f;
            if (mouse_controller.pitch < -89.0f)
                mouse_controller.pitch = -89.0f;

            fp_camera.direction.x = cos(glm::radians(mouse_controller.yaw)) * cos(glm::radians(mouse_controller.pitch));
            fp_camera.direction.y = sin(glm::radians(mouse_controller.pitch));
            fp_camera.direction.z = sin(glm::radians(mouse_controller.yaw)) * cos(glm::radians(mouse_controller.pitch));
            fp_camera.direction = glm::normalize(fp_camera.direction);

            mouse_controller.last_cursor_x = float(e->x);
            mouse_controller.last_cursor_y = float(e->y);
        }

        return false;
    }
};
