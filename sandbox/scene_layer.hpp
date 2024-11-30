#pragma once

#include "kione2D.hpp"

#include "components.hpp"
#include "core/scene.hpp"
#include "rendering/model.hpp"

#include "skybox.hpp"
#include <imgui.h>
#include <imgui_stdlib.h>

class SceneLayer : public k2::Layer {
    k2::Window& window;

    k2::Scene scene;

    k2::Program pbr_program {};
    k2::Program skybox_program {};

    SkyBox skybox {};

    struct MouseController {
        float pitch = 0.0f;
        float yaw = -90.0f;
        float sensitivity = 0.1f;

        float last_cursor_x {};
        float last_cursor_y {};
    };

    MouseController mouse_controller {};
    std::string model_path{"res/models/backpack.obj"};

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
        pbr_program = program_loader("res/shaders/pbr_vs.glsl", "res/shaders/pbr_fs.glsl");
        skybox_program = program_loader("res/shaders/skybox_vs.glsl", "res/shaders/skybox_fs.glsl");

        setup_scene();
    }

    SceneLayer(const SceneLayer&) = delete;

    SceneLayer& operator=(const SceneLayer&) = delete;

    void update(float dt) override {
        float camera_speed = 3.0f;

        using k2::KeyboardDevice;

        auto& fp_camera = scene.registry.ctx().get<FPCamera>();
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
        render_game_controls();

        auto& fp_camera = scene.registry.ctx().get<FPCamera>();
        fp_camera.update();
        auto view_mat = fp_camera.camera.get_view();
        auto projection_mat = fp_camera.camera.get_projection();

        skybox_program.use()
            .set_uniform("view", glm::mat4(glm::mat3(view_mat)))
            .set_uniform("projection", projection_mat);
        skybox.draw(skybox_program);

        scene.registry.view<k2::TransformComponent, k2::Model>().each([&](auto, auto& transform, auto& model) {
            auto model_mat = transform.get_matrix();

            pbr_program.use()
                .set_uniform("model", model_mat)
                .set_uniform("view", view_mat)
                .set_uniform("projection", projection_mat);
            model.draw(pbr_program);
        });
        
    }

    bool handle_event(const k2::Event* ev) override {
        using namespace k2::literals;

        auto& fp_camera = scene.registry.ctx().get<FPCamera>();

        HANDLE_EVENT(k2::CursorPositionEvent, ev, event, {
            auto diffX = -float(mouse_controller.last_cursor_x - event.x);
            auto diffY = float(mouse_controller.last_cursor_y - event.y);

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

            mouse_controller.last_cursor_x = float(event.x);
            mouse_controller.last_cursor_y = float(event.y);
        })

        return false;
    }

private:
    void setup_scene() {
        scene.registry.ctx().emplace<FPCamera>(FPCamera {
            .camera { .position { 0.f, 0.f, 6.f },
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
        });

        skybox.load({ "res/textures/skybox/right.png", "res/textures/skybox/left.png", "res/textures/skybox/bottom.png",
            "res/textures/skybox/top.png", "res/textures/skybox/front.png", "res/textures/skybox/back.png" });
        setup_scene_model();
    }

    void setup_scene_model() {
        for (auto entity: scene.registry.view<k2::Model>()) {
            scene.registry.destroy(entity);
        }
        
        auto backpack = scene.registry.create();
        scene.registry.emplace<k2::Model>(backpack, model_path);
        auto& transform = scene.registry.emplace<k2::TransformComponent>(backpack);
        transform.orientation = glm::rotate(transform.orientation, 3.1415f, glm::vec3(0.f, 1.f, 0.f));
    }

    void render_game_controls() {
        ImGui::Begin("Game Controls");
        ImGui::InputText("Model Path", &model_path);
        if (ImGui::Button("Load Model")) {
            setup_scene_model();
        }
        ImGui::End();
    }

};
