#include "rendering/batcher2D.hpp"

#include <cassert>
#include <numeric>
#include <unordered_set>

namespace k2 {

Batcher2D::Batcher2D() {
    vbo = VertexBuffer { max_vertices * sizeof(ShaderInput) };
    ebo = IndexBuffer { max_vertices * sizeof(std::uint32_t) };
    vao.apply({ {
                    .buffer = vbo.get(),
                    .attribute_trait {
                        .location = 0,
                        .data_type = ShaderDataType::Float3,
                        .offset = offsetof(ShaderInput, position),
                    },
                },
                  {
                      .buffer = vbo.get(),
                      .attribute_trait {
                          .location = 1,
                          .data_type = ShaderDataType::Float4,
                          .offset = offsetof(ShaderInput, color),
                      },
                  },
                  {
                      .buffer = vbo.get(),
                      .attribute_trait {
                          .location = 2,
                          .data_type = ShaderDataType::Float2,
                          .offset = offsetof(ShaderInput, texture_coordinate),
                      },
                  },
                  {
                      .buffer = vbo.get(),
                      .attribute_trait {
                          .location = 3,
                          .data_type = ShaderDataType::Float,
                          .offset = offsetof(ShaderInput, texture),
                      },
                  } },
        sizeof(ShaderInput), ebo.get());
}

void Batcher2D::begin(const Pass& new_pass) {
    assert(!pass);
    pass = new_pass;
}

void Batcher2D::submit(
    std::span<const Vertex2D> new_vertices, std::span<const std::uint32_t> new_indices, const glm::mat4& transform) {
    assert(pass);

    std::unordered_set<ResourceID> textures_new {};
    for (const auto& vertex : new_vertices) {
        textures_new.insert(vertex.texture);
    }
    std::size_t total_textures = texture_unit_map.size();
    for (auto& texture : textures_new) {
        total_textures += !texture_unit_map.contains(texture);
    }

    if (total_textures > max_textures || vertices.size() + new_vertices.size() > max_vertices
        || indices.size() + new_indices.size() > max_vertices) {
        flush();
    }

    for (auto& texture : textures_new) {
        if (!texture_unit_map.contains(texture)) {
            auto next_index = texture_unit_map.size();
            texture_unit_map[texture] = std::uint32_t(next_index);
        }
    }

    for (const auto& vertex : new_vertices) {
        vertices.push_back(ShaderInput { .position = glm::vec3(transform * glm::vec4(vertex.position, 1.0f)),
            .color = vertex.color,
            .texture_coordinate = vertex.texture_coordinate,
            .texture = float(texture_unit_map[vertex.texture]) });
    }

    auto offset = std::uint32_t(vertices.size() - new_vertices.size());
    for (auto index : new_indices) {
        indices.push_back(index + offset);
    }
}

void Batcher2D::end() {
    assert(pass);
    flush();
    pass.reset();
}

void Batcher2D::flush() {
    if (vertices.empty()) {
        texture_unit_map.clear();
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, pass->target->get_id());
    glViewport(pass->viewport[0], pass->viewport[1], pass->viewport[2], pass->viewport[3]);

    // Depth testing breaks blended sprites.
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(pass->blend_src, pass->blend_dst);

    std::vector<std::int32_t> texture_units(texture_unit_map.size());
    std::iota(texture_units.begin(), texture_units.end(), 0);

    for (auto& [texture_id, texture_unit_index] : texture_unit_map) {
        auto* texture = pass->resources != nullptr ? pass->resources->try_get<Texture2D>(texture_id) : nullptr;
        (texture != nullptr ? *texture : fallback_texture).bind(texture_unit_index);
    }

    pass->shader->use()
        .set_uniform("texture_list", std::span { texture_units.data(), texture_units.size() })
        .set_uniform("model", glm::mat4(1.0f))
        .set_uniform("view", pass->camera->get_view())
        .set_uniform("projection", pass->camera->get_projection());

    vao.bind();
    vbo.set(vertices.data(), sizeof(vertices[0]) * vertices.size());
    ebo.set(indices.data(), sizeof(indices[0]) * indices.size());
    glDrawElements(GL_TRIANGLES, GLsizei(indices.size()), GL_UNSIGNED_INT, nullptr);
    vao.unbind();

    texture_unit_map.clear();
    vertices.clear();
    indices.clear();
}

}
