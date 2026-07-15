#include <catch2/catch_all.hpp>

#include <sol/sol.hpp>

#include "core/script/bindings.hpp"

namespace {
sol::state make_state() {
    sol::state lua;
    lua.open_libraries(sol::lib::base);
    k2::bind_constants(lua);
    return lua;
}
}

TEST_CASE("Constant tables resolve known names to themselves") {
    auto lua = make_state();
    REQUIRE(lua.script("return Key.escape").get<std::string>() == "escape");
    REQUIRE(lua.script("return Key.kp_enter").get<std::string>() == "kp_enter");
    REQUIRE(lua.script("return MouseButton.left").get<std::string>() == "left");
    REQUIRE(lua.script("return InputState.press").get<std::string>() == "press");
    REQUIRE(lua.script("return EventType.mouse_button").get<std::string>() == "mouse_button");
}

TEST_CASE("Reading an unknown constant is an interpreter error") {
    auto lua = make_state();
    REQUIRE_FALSE(lua.safe_script("return Key.escpe", sol::script_pass_on_error).valid());
    REQUIRE_FALSE(lua.safe_script("return MouseButton.scroll", sol::script_pass_on_error).valid());
    REQUIRE_FALSE(lua.safe_script("return EventType.keydown", sol::script_pass_on_error).valid());
}

TEST_CASE("Constant tables are read-only") {
    auto lua = make_state();
    REQUIRE_FALSE(lua.safe_script("Key.escape = 'x'", sol::script_pass_on_error).valid());
    REQUIRE_FALSE(lua.safe_script("Key.new_key = 'x'", sol::script_pass_on_error).valid());
    // The value survived the attempted write.
    REQUIRE(lua.script("return Key.escape").get<std::string>() == "escape");
}

TEST_CASE("Constant table metatable is locked") {
    auto lua = make_state();
    REQUIRE(lua.script("return getmetatable(Key)").get<bool>() == false);
}
