#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "components/camera.hpp"
#include "core/resources.hpp"
#include "rendering/buffer.hpp"
#include "rendering/drawable.hpp"
#include "rendering/frame_buffer.hpp"
#include "rendering/shader.hpp"
#include "rendering/texture.hpp"
#include "rendering/vertex_array.hpp"

namespace k2 {

class Batcher2D {
public:
    struct Pass {
        const FrameBuffer* target {};
        Program* shader {};
        std::uint32_t blend_src { GL_ONE };
        std::uint32_t blend_dst { GL_ONE_MINUS_SRC_ALPHA };
        std::array<GLint, 4> viewport {};
        Camera* camera {};
        ResourceManager* resources {};
    };

    Batcher2D();

    Batcher2D(const Batcher2D&) = delete;
    Batcher2D& operator=(const Batcher2D&) = delete;

    void begin(const Pass& pass);

    void submit(std::span<const Vertex2D> vertices, std::span<const std::uint32_t> indices, const glm::mat4& transform);

    void end();

private:
    struct ShaderInput {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texture_coordinate;
        float texture;
    };

    void flush();

    std::optional<Pass> pass {};
    std::vector<ShaderInput> vertices {};
    std::vector<std::uint32_t> indices {};
    std::unordered_map<ResourceID, std::uint32_t> texture_unit_map {};
    std::size_t max_vertices = 100'000;
    // GL 4.1 guarantees only 16 fragment texture units
    std::uint32_t max_textures = 16;
    VertexArray vao;
    VertexBuffer vbo;
    IndexBuffer ebo;
    Texture2D fallback_texture = Texture2D::create_white_texture<std::uint8_t>();
};

}
