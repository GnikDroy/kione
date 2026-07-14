#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "components/camera.hpp"

using Catch::Approx;

namespace {
glm::vec3 project(k2::Camera& camera, glm::vec3 world) {
    auto clip = camera.get_view_projection() * glm::vec4 { world, 1.0f };
    return glm::vec3 { clip } / clip.w;
}
}

TEST_CASE("orthographic camera maps its bounds to clip space", "[camera]") {
    k2::Camera camera {
        .position { 0.0f, 0.0f, 1000.0f },
        .target { 0.0f, 0.0f, 0.0f },
        .up { 0.0f, 1.0f, 0.0f },
        .projection_traits { k2::Camera::OrthographicTraits {
            .left = -800.0f,
            .right = 800.0f,
            .top = 450.0f,
            .bottom = -450.0f,
            .far_clip = 0.0f,
            .near_clip = 2000.0f,
        } },
    };

    auto center = project(camera, { 0.0f, 0.0f, 0.0f });
    REQUIRE(center.x == Approx(0.0f).margin(1e-5));
    REQUIRE(center.y == Approx(0.0f).margin(1e-5));

    auto corner = project(camera, { 800.0f, 450.0f, 0.0f });
    REQUIRE(corner.x == Approx(1.0f));
    REQUIRE(corner.y == Approx(1.0f));

    auto opposite = project(camera, { -800.0f, -450.0f, 0.0f });
    REQUIRE(opposite.x == Approx(-1.0f));
    REQUIRE(opposite.y == Approx(-1.0f));
}

TEST_CASE("orthographic depth range covers world z in [-1000, 1000]", "[camera]") {
    // The engine's convention: camera at z=1000 with near=2000/far=0 sees
    // world z from -1000 (near limit) to 1000 (far limit).
    k2::Camera camera {
        .position { 0.0f, 0.0f, 1000.0f },
        .target { 0.0f, 0.0f, 0.0f },
        .up { 0.0f, 1.0f, 0.0f },
        .projection_traits { k2::Camera::OrthographicTraits {
            .left = -1.0f,
            .right = 1.0f,
            .top = 1.0f,
            .bottom = -1.0f,
            .far_clip = 0.0f,
            .near_clip = 2000.0f,
        } },
    };

    REQUIRE(project(camera, { 0.0f, 0.0f, -1000.0f }).z == Approx(-1.0f));
    REQUIRE(project(camera, { 0.0f, 0.0f, 1000.0f }).z == Approx(1.0f));
    REQUIRE(std::abs(project(camera, { 0.0f, 0.0f, 0.0f }).z) < 1.0f);
}

TEST_CASE("view matrix moves the world opposite the camera", "[camera]") {
    k2::Camera camera {
        .position { 100.0f, 50.0f, 10.0f },
        .target { 100.0f, 50.0f, 0.0f },
        .up { 0.0f, 1.0f, 0.0f },
        .projection_traits { k2::Camera::OrthographicTraits {} },
    };

    auto view_space = camera.get_view() * glm::vec4 { 100.0f, 50.0f, 0.0f, 1.0f };
    REQUIRE(view_space.x == Approx(0.0f).margin(1e-5));
    REQUIRE(view_space.y == Approx(0.0f).margin(1e-5));
    REQUIRE(view_space.z == Approx(-10.0f));
}

TEST_CASE("perspective projection matches glm reference", "[camera]") {
    k2::Camera camera {
        .projection_traits { k2::Camera::PerspectiveTraits {
            .fov = 1.0f,
            .aspect_ratio = 16.0f / 9.0f,
            .far_clip = 1000.0f,
            .near_clip = 0.1f,
        } },
    };

    auto expected = glm::perspective(1.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
    REQUIRE(camera.get_projection() == expected);
}
