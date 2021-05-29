#include <iostream>

#include "kione2D.hpp"

#include "glad/glad.h"
#include "stb_image.h"

#include "core/imgui_layer.hpp"
#include "core/rendering/shader.hpp"

#include <map>
#include <ranges>

float vertices[] = {
        -0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0f,   1.0f, 0.0f,
        -0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f, 1.0f,   0.0f, 1.0f,
         0.5f,  0.5f, 0.0f,   0.5f, 0.6f, 0.8f, 1.0f,   1.0f, 1.0f,
};

int indices[] = {
        0, 1, 2,
        1, 2, 3
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


        int width{}, height{}, channels{};
        stbi_set_flip_vertically_on_load(true);
        auto tex_data = stbi_load("res/texture.jpg", &width, &height, &channels, 0);


        GLuint tex{};
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(tex_data);


        auto vertex_shader = k2::Shader(GL_VERTEX_SHADER, fs::path("res/vs.glsl"));

        if (!vertex_shader.good()){
            k2::Logger::app->critical(vertex_shader.error_msg().value());
        }
        auto fragment_shader = k2::Shader(GL_FRAGMENT_SHADER, fs::path("res/fs.glsl"));

        if (!fragment_shader.good()){
            k2::Logger::app->critical(fragment_shader.error_msg().value());
        }

        k2::Program shader_program{vertex_shader, fragment_shader};
        shader_program.link();

        if (!shader_program.good()){
            k2::Logger::app->critical(shader_program.error_msg().value());
        }

        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        GLuint ebo;
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), nullptr);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*) (3 * sizeof(float)));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*) (7 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        while (running) {

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

                for (auto &layer: std::views::reverse(layers)) {
                    if (layer->handle_event(event.get())) { break; }
                }
            }

            // Render
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
//          glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex);

            glBindVertexArray(vao);
            shader_program.use();
            glUniform1i(glGetUniformLocation(shader_program.handle, "tex"), 0);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
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