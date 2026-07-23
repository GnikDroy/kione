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

TEST_CASE("screen_to_world / world_to_screen round-trip through a viewport rect", "[camera]") {
    k2::Camera camera {
        .position { 0.0f, 0.0f, 1000.0f },
        .target { 0.0f, 0.0f, 0.0f },
        .up { 0.0f, 1.0f, 0.0f },
        .projection_traits { k2::Camera::OrthographicTraits {
            .left = -640.0f,
            .right = 640.0f,
            .top = 360.0f,
            .bottom = -360.0f,
            .far_clip = 0.0f,
            .near_clip = 2000.0f,
        } },
    };
    k2::SceneView window { .camera = camera, .viewport = { .x = 0.0f, .y = 0.0f, .w = 1280.0f, .h = 720.0f } };

    auto center = window.screen_to_world({ 640.0f, 360.0f });
    REQUIRE(center.x == Approx(0.0f).margin(1e-3));
    REQUIRE(center.y == Approx(0.0f).margin(1e-3));

    // screen origin is top-left, y-down; world y is up
    auto top_left = window.screen_to_world({ 0.0f, 0.0f });
    REQUIRE(top_left.x == Approx(-640.0f));
    REQUIRE(top_left.y == Approx(360.0f));

    // an editor-style viewport image offset inside the window
    k2::SceneView offset { .camera = camera, .viewport = { .x = 100.0f, .y = 50.0f, .w = 640.0f, .h = 360.0f } };
    auto offset_center = offset.screen_to_world({ 100.0f + 320.0f, 50.0f + 180.0f });
    REQUIRE(offset_center.x == Approx(0.0f).margin(1e-3));
    REQUIRE(offset_center.y == Approx(0.0f).margin(1e-3));

    auto screen = offset.world_to_screen({ -640.0f, 360.0f });
    REQUIRE(screen.x == Approx(100.0f));
    REQUIRE(screen.y == Approx(50.0f));

    auto round_trip = window.screen_to_world(window.world_to_screen({ 123.0f, -217.0f }));
    REQUIRE(round_trip.x == Approx(123.0f));
    REQUIRE(round_trip.y == Approx(-217.0f));
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

namespace {
// 16:9 design view (aspect 1.777...).
k2::Camera design_camera(k2::ScaleMode mode) {
    k2::Camera camera { .projection_traits { k2::Camera::OrthographicTraits {
        .left = -800.0f, .right = 800.0f, .top = 450.0f, .bottom = -450.0f } } };
    camera.scale_mode = mode;
    return camera;
}
const k2::Camera::OrthographicTraits& ortho_of(const k2::Camera& camera) {
    return std::get<k2::Camera::OrthographicTraits>(camera.projection_traits);
}
}

TEST_CASE("letterbox_fit centers a design-aspect rect inside the surface", "[camera][scale]") {
    // Equal aspect: fills the surface.
    auto exact = k2::letterbox_fit(1600.0f, 900.0f, 16.0f / 9.0f);
    REQUIRE(exact.x == Approx(0.0f));
    REQUIRE(exact.y == Approx(0.0f));
    REQUIRE(exact.w == Approx(1600.0f));
    REQUIRE(exact.h == Approx(900.0f));

    // Wider surface -> pillarbox (bars left/right), full height.
    auto pillar = k2::letterbox_fit(2000.0f, 900.0f, 16.0f / 9.0f);
    REQUIRE(pillar.w == Approx(1600.0f));
    REQUIRE(pillar.h == Approx(900.0f));
    REQUIRE(pillar.x == Approx(200.0f));
    REQUIRE(pillar.y == Approx(0.0f));

    // Taller surface -> letterbox (bars top/bottom), full width.
    auto letter = k2::letterbox_fit(1600.0f, 1200.0f, 16.0f / 9.0f);
    REQUIRE(letter.w == Approx(1600.0f));
    REQUIRE(letter.h == Approx(900.0f));
    REQUIRE(letter.x == Approx(0.0f));
    REQUIRE(letter.y == Approx(150.0f));
}

TEST_CASE("for_surface: Stretch leaves bounds and uses the full surface", "[camera][scale]") {
    auto resolved = design_camera(k2::ScaleMode::Stretch).for_surface(2000.0f, 900.0f);
    const auto& bounds = ortho_of(resolved.camera);
    REQUIRE(bounds.left == Approx(-800.0f));
    REQUIRE(bounds.right == Approx(800.0f));
    REQUIRE(resolved.viewport.w == Approx(2000.0f));
    REQUIRE(resolved.viewport.h == Approx(900.0f));
}

TEST_CASE("for_surface: FitHeight keeps vertical extent, widens horizontally", "[camera][scale]") {
    auto resolved = design_camera(k2::ScaleMode::FitHeight).for_surface(2000.0f, 900.0f);
    const auto& bounds = ortho_of(resolved.camera);
    REQUIRE(bounds.top == Approx(450.0f));
    REQUIRE(bounds.bottom == Approx(-450.0f));
    REQUIRE(bounds.left == Approx(-1000.0f)); // 450 * (2000/900)
    REQUIRE(bounds.right == Approx(1000.0f));
    REQUIRE(resolved.viewport.w == Approx(2000.0f)); // full surface, no bars
}

TEST_CASE("for_surface: FitWidth keeps horizontal extent, grows vertically", "[camera][scale]") {
    auto resolved = design_camera(k2::ScaleMode::FitWidth).for_surface(1600.0f, 1200.0f);
    const auto& bounds = ortho_of(resolved.camera);
    REQUIRE(bounds.left == Approx(-800.0f));
    REQUIRE(bounds.right == Approx(800.0f));
    REQUIRE(bounds.top == Approx(600.0f)); // 800 / (1600/1200)
    REQUIRE(bounds.bottom == Approx(-600.0f));
}

TEST_CASE("for_surface: Expand reveals more on the longer axis", "[camera][scale]") {
    // Wider than design -> behaves like FitHeight.
    auto wide = design_camera(k2::ScaleMode::Expand).for_surface(2000.0f, 900.0f);
    REQUIRE(ortho_of(wide.camera).right == Approx(1000.0f));
    REQUIRE(ortho_of(wide.camera).top == Approx(450.0f));
    // Taller than design -> behaves like FitWidth.
    auto tall = design_camera(k2::ScaleMode::Expand).for_surface(900.0f, 1600.0f);
    REQUIRE(ortho_of(tall.camera).right == Approx(800.0f));
    REQUIRE(ortho_of(tall.camera).top == Approx(800.0f / (900.0f / 1600.0f)));
}

TEST_CASE("for_surface: Letterbox keeps bounds and returns the centered sub-rect", "[camera][scale]") {
    auto resolved = design_camera(k2::ScaleMode::Letterbox).for_surface(2000.0f, 900.0f);
    const auto& bounds = ortho_of(resolved.camera);
    REQUIRE(bounds.left == Approx(-800.0f)); // unchanged
    REQUIRE(bounds.right == Approx(800.0f));
    REQUIRE(resolved.viewport.x == Approx(200.0f)); // pillarbox sub-rect
    REQUIRE(resolved.viewport.w == Approx(1600.0f));
    REQUIRE(resolved.viewport.h == Approx(900.0f));
}
