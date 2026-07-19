#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "rendering/shapes.hpp"

#include <algorithm>

using Catch::Approx;
using namespace k2::shapes;

namespace {
bool indices_in_range(const Mesh& mesh) {
    return std::ranges::all_of(mesh.indices, [&](auto index) { return index < mesh.positions.size(); });
}
}

TEST_CASE("line_mesh is a width-parameterized quad", "[shapes]") {
    auto mesh = line_mesh({ 0.0f, 0.0f }, { 10.0f, 0.0f }, 2.0f);
    REQUIRE(mesh.positions.size() == 4);
    REQUIRE(mesh.indices.size() == 6);
    REQUIRE(indices_in_range(mesh));
    // Perpendicular expansion of +/- width/2 on y for a horizontal line.
    REQUIRE(mesh.positions[0].y == Approx(1.0f));
    REQUIRE(mesh.positions[1].y == Approx(-1.0f));
    REQUIRE(mesh.positions[2].x == Approx(10.0f));

    REQUIRE(line_mesh({ 3.0f, 3.0f }, { 3.0f, 3.0f }, 2.0f).positions.empty()); // zero length
    REQUIRE(line_mesh({ 0.0f, 0.0f }, { 1.0f, 0.0f }, 0.0f).positions.empty()); // zero width
}

TEST_CASE("rect meshes cover and outline the same corners", "[shapes]") {
    auto filled = rect_mesh({ 10.0f, 20.0f }, { 4.0f, 6.0f });
    REQUIRE(filled.positions.size() == 4);
    REQUIRE(filled.indices.size() == 6);
    REQUIRE(filled.positions[0].x == Approx(8.0f));
    REQUIRE(filled.positions[2].y == Approx(23.0f));

    auto outline = rect_outline_mesh({ 0.0f, 0.0f }, { 10.0f, 10.0f }, 2.0f);
    REQUIRE(outline.positions.size() == 8); // corners shared through indices, not duplicated
    REQUIRE(outline.indices.size() == 24);
    REQUIRE(indices_in_range(outline));
    REQUIRE(outline.positions[0].x == Approx(-6.0f)); // outer = half + thickness/2
    REQUIRE(outline.positions[4].x == Approx(-4.0f)); // inner = half - thickness/2
}

TEST_CASE("circle_segment_count follows the chordal-error policy", "[shapes]") {
    REQUIRE(circle_segment_count(0.1f) == 12); // tiny radii clamp low
    REQUIRE(circle_segment_count(100000.0f) == 128); // huge radii clamp high
    auto small = circle_segment_count(50.0f);
    auto large = circle_segment_count(300.0f);
    REQUIRE(small >= 12);
    REQUIRE(large <= 128);
    REQUIRE(large >= small); // monotone in radius
}

TEST_CASE("circle meshes close by index wrap-around", "[shapes]") {
    auto filled = circle_mesh({ 0.0f, 0.0f }, 10.0f, 16);
    REQUIRE(filled.positions.size() == 17); // center + ring, no duplicated seam vertex
    REQUIRE(filled.indices.size() == 16 * 3);
    REQUIRE(indices_in_range(filled));
    // The last triangle wraps to perimeter index 1.
    REQUIRE(filled.indices[filled.indices.size() - 1] == 1);

    auto ring = circle_outline_mesh({ 0.0f, 0.0f }, 10.0f, 4.0f, 16);
    REQUIRE(ring.positions.size() == 32);
    REQUIRE(ring.indices.size() == 16 * 6);
    REQUIRE(indices_in_range(ring));
    REQUIRE(glm::length(ring.positions[0]) == Approx(12.0f)); // outer radius
    REQUIRE(glm::length(ring.positions[16]) == Approx(8.0f)); // inner radius
}

TEST_CASE("polygon and polyline meshes", "[shapes]") {
    const glm::vec2 triangle[] = { { 0.0f, 0.0f }, { 10.0f, 0.0f }, { 5.0f, 8.0f } };
    auto fan = polygon_mesh(triangle);
    REQUIRE(fan.positions.size() == 3);
    REQUIRE(fan.indices.size() == 3);
    REQUIRE(polygon_mesh(std::span<const glm::vec2> { triangle, 2 }).positions.empty());

    auto open = polyline_mesh(triangle, 1.0f, false);
    REQUIRE(open.positions.size() == 8); // 2 segments x 4
    auto closed = polyline_mesh(triangle, 1.0f, true);
    REQUIRE(closed.positions.size() == 12); // 3 segments x 4
    REQUIRE(indices_in_range(closed));
}
