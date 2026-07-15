#include <catch2/catch_all.hpp>

#include "core/script/key_names.hpp"

using KeyCode = k2::KeyboardDevice::KeyCode;

TEST_CASE("Every KeyCode is named and round-trips") {
    int named = 0;
    for (int code = int(KeyCode::key_space); code <= int(KeyCode::key_last); code++) {
        auto name = k2::key_name_from(KeyCode(code));
        if (!name) {
            continue;
        }
        named++;
        REQUIRE(k2::key_code_from(*name) == KeyCode(code));
    }
    REQUIRE(named == 120);
}

TEST_CASE("Key names follow the enum identifiers") {
    REQUIRE(k2::key_code_from("a") == KeyCode::key_a);
    REQUIRE(k2::key_code_from("9") == KeyCode::key_9);
    REQUIRE(k2::key_code_from("escape") == KeyCode::key_escape);
    REQUIRE(k2::key_code_from("kp_enter") == KeyCode::key_kp_enter);
    REQUIRE(k2::key_code_from("left_super") == KeyCode::key_left_super);
    REQUIRE(k2::key_name_from(KeyCode::key_grave_accent) == "grave_accent");
    REQUIRE(k2::key_name_from(KeyCode::key_menu) == "menu");
    REQUIRE_FALSE(k2::key_name_from(KeyCode::key_unknown).has_value());
}

TEST_CASE("Unknown key names have no code") {
    REQUIRE_FALSE(k2::key_code_from("not_a_key").has_value());
    REQUIRE_FALSE(k2::key_code_from("").has_value());
    REQUIRE_FALSE(k2::key_code_from("A").has_value());
}
