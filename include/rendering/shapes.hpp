#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include <glm/glm.hpp>

namespace k2::shapes {

struct Mesh {
    std::vector<glm::vec2> positions {};
    std::vector<std::uint32_t> indices {};
};

Mesh line_mesh(glm::vec2 a, glm::vec2 b, float width);

Mesh rect_mesh(glm::vec2 center, glm::vec2 size);

Mesh rect_outline_mesh(glm::vec2 center, glm::vec2 size, float thickness);

int circle_segment_count(float radius, float max_error = 0.25f);

Mesh circle_mesh(glm::vec2 center, float radius, int segments);

Mesh circle_outline_mesh(glm::vec2 center, float radius, float thickness, int segments);

Mesh polygon_mesh(std::span<const glm::vec2> points);

Mesh polyline_mesh(std::span<const glm::vec2> points, float width, bool closed);

}
