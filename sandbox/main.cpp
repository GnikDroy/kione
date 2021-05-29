#include <iostream>

#include "kione2D.hpp"

#include "glad/glad.h"
#include "stb_image.h"

#include "core/imgui_layer.hpp"
#include "core/rendering/shader.hpp"
#include "core/rendering/image.hpp"


#pragma warning(disable : 4201)
#define GLM_FORCE_CXX2A
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <map>
#include <ranges>

float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f,   1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f,   1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f,   1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f,    0.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f,    1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f,    1.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f,    1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f,    0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f,    0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 0.0f, 0.0f,   1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,   0.0f, 0.0f, 0.0f, 0.0f,   1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,   0.0f, 0.0f, 0.0f, 0.0f,   0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,   0.0f, 0.0f, 0.0f, 0.0f,   0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 0.0f, 0.0f,   1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, 0.0f, 0.0f,   0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   0.0f, 0.0f, 0.0f, 0.0f,   1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 0.0f, 0.0f,   1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 0.0f, 0.0f,   1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, 0.0f, 0.0f,   0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,   0.0f, 0.0f, 0.0f, 0.0f,    0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,   0.0f, 0.0f, 0.0f, 0.0f,    1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 0.0f, 0.0f,    1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 0.0f, 0.0f,    1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 0.0f, 0.0f,    0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,   0.0f, 0.0f, 0.0f, 0.0f,    0.0f, 1.0f
};

class Sandbox : public k2::App {
public:
    k2::Window window;
    bool running = true;
    std::vector<std::unique_ptr<k2::Layer>> layers;

    Sandbox() : App(), window{} {
        k2::Logger::app->info("Sandbox application started.");
    }

    void run() override {
        using namespace k2::literals;
        namespace fs = std::filesystem;

        layers.push_back(std::make_unique<k2::ImguiLayer>(window));



        k2::Image image{"res/texture.jpg"};
        if (!image) {
            k2::Logger::app->critical("Couldn't load image.");
        }

        GLuint tex{};
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width, image.height, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data);
        glGenerateMipmap(GL_TEXTURE_2D);


        auto vertex_shader = k2::Shader(GL_VERTEX_SHADER, fs::path("res/vs.glsl"));

        if (!vertex_shader){
            k2::Logger::app->critical(vertex_shader.error_msg().value());
        }
        auto fragment_shader = k2::Shader(GL_FRAGMENT_SHADER, fs::path("res/fs.glsl"));

        if (!fragment_shader){
            k2::Logger::app->critical(fragment_shader.error_msg().value());
        }

        k2::Program shader_program{vertex_shader, fragment_shader};
        shader_program.link();

        if (!shader_program){
            k2::Logger::app->critical(shader_program.error_msg().value());
        }

        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), nullptr);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*) (3 * sizeof(float)));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*) (7 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        glEnable(GL_DEPTH_TEST);

        glm::vec3 cube_positions[] = {
                { 0.0f,  0.0f, 0.0f},
                { 1.0f,  2.0f, -2.0f},
                { 1.0f, -2.0f, 2.0f},
        };

        glm::vec3 camera_position(0.0f, 0.0f,  -3.0f);
        glm::vec3 camera_offset(0.0f, 0.0f, 1.0f);
        glm::vec3 camera_up(0.0f, 1.0f,  0.0f);
        float camera_speed = 5.0f;

        auto current_frame = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        auto last_frame = current_frame;
        while (running) {
            current_frame = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            auto dt = float(current_frame-last_frame) / 1000.0f;
            last_frame = current_frame;

            // Populate event buffer.
            window.update();

            // Handle all events in layers.
            for (; !window.events.empty(); window.events.pop()) {
                const auto event = std::move(window.events.front());

                if (event->type == "WindowFramebufferResizeEvent"_fnv1a) {
                    auto e = reinterpret_cast<k2::WindowFramebufferResizeEvent *>(event.get());
                    glViewport(0, 0, e->width/2, e->height/2);
                }
                if (event->type == "WindowCloseEvent"_fnv1a) {
                    k2::Logger::app->info("Window Close Event Received.");
                    running = false;
                }

                if (window.keyboard.get_state(k2::KeyboardDevice::KeyCode::key_w) == k2::KeyboardDevice::KeyState::press) {
                    camera_position += camera_speed * dt * camera_offset;
                } else if (window.keyboard.get_state(k2::KeyboardDevice::KeyCode::key_s) == k2::KeyboardDevice::KeyState::press) {
                    camera_position -= camera_speed * dt * camera_offset;
                } else if (window.keyboard.get_state(k2::KeyboardDevice::KeyCode::key_a) == k2::KeyboardDevice::KeyState::press) {
                    camera_position -= glm::normalize(glm::cross(camera_offset, camera_up)) * camera_speed * dt;
                } else if (window.keyboard.get_state(k2::KeyboardDevice::KeyCode::key_d) == k2::KeyboardDevice::KeyState::press) {
                    camera_position += glm::normalize(glm::cross(camera_offset, camera_up)) * dt * camera_speed;
                }

        for (auto &layer: std::views::reverse(layers)) {
                    if (layer->handle_event(event.get())) { break; }
                }
            }

            // Render
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex);

            glBindVertexArray(vao);
            shader_program.use();
            glUniform1i(glGetUniformLocation(shader_program.handle, "tex"), 0);

            auto view = glm::lookAt(camera_position, camera_offset + camera_position, camera_up);

            glm::mat4 projection;
            projection = glm::perspective(glm::radians(45.0f), window.get_width() / (float) window.get_height(), 0.1f, 100.0f);

            glUniformMatrix4fv(glGetUniformLocation(shader_program.handle, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(shader_program.handle, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            for(auto & position : cube_positions)
            {
                auto model = glm::translate(glm::mat4(1.0f), position);
                model = glm::rotate(model, glm::radians(15.0f), glm::vec3(1.0f, 1.0f, 0.0f));
                glUniformMatrix4fv(glGetUniformLocation(shader_program.handle, "model"), 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLES, 0, (GLsizei) std::size(vertices));
            }

            for (auto &layer: layers) { layer->render(); }
        }
        k2::Logger::app->info("Sandbox application run() end.");
    }

    ~Sandbox() override {
        k2::Logger::app->info("Sandbox application stopped.");
    }
};

std::unique_ptr<k2::App> create_app() {
    return std::unique_ptr<k2::App>{std::make_unique<Sandbox>()};
}