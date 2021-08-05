#pragma once
#include <array>

#include "rendering/buffer.hpp"
#include "rendering/texture.hpp"
#include "rendering/vertex_array.hpp"

static inline const float vertices[] = { -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
    -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
    1.0f,

    1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,

    -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
    1.0f };

class SkyBox {
    k2::VertexArray vao;
    k2::VertexBuffer vbo;
    k2::TextureCube cube;

public:
    SkyBox() = default;
    explicit SkyBox(const std::array<std::filesystem::path, 6>& paths) { load(paths); }

    SkyBox(const SkyBox&) = delete;
    SkyBox& operator=(const SkyBox&) = delete;

    SkyBox& load(const std::array<std::filesystem::path, 6>& paths) {
        if (!cube) {
            cube = k2::TextureCube(paths);
            vbo = k2::VertexBuffer(std::size(vertices) * sizeof(float));

            vao.bind();
            vbo.bind();
            vbo.set(vertices, std::size(vertices) * sizeof(float));

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);

            k2::VertexBuffer::unbind();
            k2::VertexArray::unbind();
        }
        return *this;
    }

    void draw(const k2::Program& program) {
        if (cube) {
            glDepthMask(GL_FALSE);
            vao.bind();
            program.use();
            glBindTexture(GL_TEXTURE_CUBE_MAP, cube.id);
            glDrawArrays(GL_TRIANGLES, 0, (GLint)(std::size(vertices) / 3));
            k2::VertexArray::unbind();
            glDepthMask(GL_TRUE);
        }
    }
};
