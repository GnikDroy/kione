#include "rendering/shapes.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace k2::shapes {

Mesh line_mesh(glm::vec2 a, glm::vec2 b, float width) {
    auto direction = b - a;
    auto length = glm::length(direction);
    if (length <= 0.0f || width <= 0.0f) {
        return {};
    }
    auto normal = glm::vec2 { -direction.y, direction.x } / length * (width / 2.0f);
    return {
        .positions = { a + normal, a - normal, b + normal, b - normal },
        .indices = { 0, 1, 2, 1, 3, 2 },
    };
}

Mesh rect_mesh(glm::vec2 center, glm::vec2 size) {
    auto half = size / 2.0f;
    return {
        .positions = { center + glm::vec2 { -half.x, -half.y }, center + glm::vec2 { half.x, -half.y },
            center + glm::vec2 { half.x, half.y }, center + glm::vec2 { -half.x, half.y } },
        .indices = { 0, 1, 2, 0, 2, 3 },
    };
}

Mesh rect_outline_mesh(glm::vec2 center, glm::vec2 size, float thickness) {
    auto outer = size / 2.0f + thickness / 2.0f;
    auto inner = size / 2.0f - thickness / 2.0f;
    inner = glm::max(inner, glm::vec2 { 0.0f });

    Mesh mesh;
    for (auto half : { outer, inner }) {
        mesh.positions.push_back(center + glm::vec2 { -half.x, -half.y });
        mesh.positions.push_back(center + glm::vec2 { half.x, -half.y });
        mesh.positions.push_back(center + glm::vec2 { half.x, half.y });
        mesh.positions.push_back(center + glm::vec2 { -half.x, half.y });
    }
    for (std::uint32_t corner = 0; corner < 4; corner++) {
        auto next = (corner + 1) % 4;
        mesh.indices.insert(mesh.indices.end(),
            { corner, next, 4 + next, corner, 4 + next, 4 + corner });
    }
    return mesh;
}

int circle_segment_count(float radius, float max_error) {
    if (radius <= max_error) {
        return 12;
    }
    float theta = std::acos(1.0f - max_error / radius);
    auto segments = int(std::ceil(std::numbers::pi_v<float> / theta));
    return std::clamp(segments, 12, 128);
}

namespace {
    std::vector<glm::vec2> ring_positions(glm::vec2 center, float radius, int segments) {
        std::vector<glm::vec2> positions;
        positions.reserve(std::size_t(segments));
        for (int i = 0; i < segments; i++) {
            float angle = 2.0f * std::numbers::pi_v<float> * float(i) / float(segments);
            positions.push_back(center + radius * glm::vec2 { std::cos(angle), std::sin(angle) });
        }
        return positions;
    }
}

Mesh circle_mesh(glm::vec2 center, float radius, int segments) {
    if (radius <= 0.0f || segments < 3) {
        return {};
    }
    Mesh mesh;
    mesh.positions.push_back(center);
    auto ring = ring_positions(center, radius, segments);
    mesh.positions.insert(mesh.positions.end(), ring.begin(), ring.end());
    for (std::uint32_t i = 0; i < std::uint32_t(segments); i++) {
        mesh.indices.insert(mesh.indices.end(), { 0, 1 + i, 1 + (i + 1) % std::uint32_t(segments) });
    }
    return mesh;
}

Mesh circle_outline_mesh(glm::vec2 center, float radius, float thickness, int segments) {
    if (radius <= 0.0f || thickness <= 0.0f || segments < 3) {
        return {};
    }
    Mesh mesh;
    auto outer = ring_positions(center, radius + thickness / 2.0f, segments);
    auto inner = ring_positions(center, std::max(radius - thickness / 2.0f, 0.0f), segments);
    mesh.positions.insert(mesh.positions.end(), outer.begin(), outer.end());
    mesh.positions.insert(mesh.positions.end(), inner.begin(), inner.end());
    auto count = std::uint32_t(segments);
    for (std::uint32_t i = 0; i < count; i++) {
        auto next = (i + 1) % count;
        mesh.indices.insert(mesh.indices.end(), { i, next, count + next, i, count + next, count + i });
    }
    return mesh;
}

Mesh polygon_mesh(std::span<const glm::vec2> points) {
    if (points.size() < 3) {
        return {};
    }
    Mesh mesh;
    mesh.positions.assign(points.begin(), points.end());
    for (std::uint32_t i = 1; i + 1 < std::uint32_t(points.size()); i++) {
        mesh.indices.insert(mesh.indices.end(), { 0, i, i + 1 });
    }
    return mesh;
}

Mesh polyline_mesh(std::span<const glm::vec2> points, float width, bool closed) {
    if (points.size() < 2) {
        return {};
    }
    Mesh mesh;
    auto segment_count = points.size() - (closed ? 0 : 1);
    for (std::size_t i = 0; i < segment_count; i++) {
        auto segment = line_mesh(points[i], points[(i + 1) % points.size()], width);
        auto offset = std::uint32_t(mesh.positions.size());
        mesh.positions.insert(mesh.positions.end(), segment.positions.begin(), segment.positions.end());
        for (auto index : segment.indices) {
            mesh.indices.push_back(index + offset);
        }
    }
    return mesh;
}

}
