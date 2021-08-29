#include <catch2/catch.hpp>

#include "core/imgui_theme.hpp"

static bool ImVec4EqualsExact(const ImVec4& a, const ImVec4& b) {
    // Exact floating point equals. Is OK in this case.
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

TEST_CASE("HexColorToImVec4") {
    std::vector<const char*> invalid = {
        "",
        "#",
        "abc",
        "aabbccdd",
        "#rrbbccddee",
        "#aabbccddee",
    };

    std::vector<std::pair<const char*, ImVec4>> valid = {
        { "#aabbccdd", { 0xaa / 255.f, 0xbb / 255.f, 0xcc / 255.f, 0xdd / 255.f } },
        { "#ff", { 0, 0, 0, 1 } },
        { "#ffff", { 0, 0, 1, 1 } },
        { "#ffffff", { 0, 1, 1, 1 } },
        { "#112233fa", { 0x11 / 255.f, 0x22 / 255.f, 0x33 / 255.f, 0xfa / 255.f } },
    };

    for (auto& i : invalid) {
        REQUIRE_THROWS_AS(k2::Imgui::HexColorToImVec4(i), std::invalid_argument);
    }
    for (auto& [hex, color] : valid) {
        REQUIRE(ImVec4EqualsExact(k2::Imgui::HexColorToImVec4(hex), color));
    }
}