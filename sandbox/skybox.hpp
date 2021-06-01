#pragma once
#include <array>

#include "core/rendering/texture.hpp"
#include "core/rendering/vertex_array.hpp"
#include "core/rendering/buffer.hpp"

static inline const float vertices[] = {
-1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
 1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,

-1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
-1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,

 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
 1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f, 1.0f, -1.0f, -1.0f,

-1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
 1.0f,  1.0f,  1.0f, 1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,

-1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f,
 1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,

-1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f, -1.0f,
 1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f,  1.0f
};

class SkyBox {
    k2::VertexArray vao;
    k2::VertexBuffer vbo;
    k2::TextureCube cube;
public:
    SkyBox() = default;
    SkyBox(const std::array<std::filesystem::path, 6>& paths) { load(paths); }

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

            vbo.unbind();
            vao.unbind();
        }
        return *this;
    }

    void draw(const k2::Program& program) {
        if (cube){
            glDepthMask(GL_FALSE);
            vao.bind();
            program.use();
            glBindTexture(GL_TEXTURE_CUBE_MAP, cube.id);
            glDrawArrays(GL_TRIANGLES, 0, (GLint)(std::size(vertices) / 3));
            vao.unbind();
            glDepthMask(GL_TRUE);

        }
    }
};