#include <catch2/catch_all.hpp>

#include <sol/sol.hpp>

#include "core/script/bindings.hpp"

namespace {
sol::state make_state() {
    sol::state lua;
    lua.open_libraries(sol::lib::base);
    return lua;
}
}

TEST_CASE("reset_table_to_baseline wipes only script-added keys") {
    auto lua = make_state();
    lua.script("engine_api = 1"); // stands in for a bound engine global
    auto baseline = k2::table_string_keys(lua.globals());
    REQUIRE(baseline.contains("engine_api"));

    lua.script("game = { gold = 12 }"); // a script's leftover global
    REQUIRE(k2::table_string_keys(lua.globals()).contains("game"));

    k2::reset_table_to_baseline(lua.globals(), baseline);

    REQUIRE(lua["game"] == sol::lua_nil);
    REQUIRE(lua["engine_api"] == 1); // baseline entry preserved
    REQUIRE(lua["print"].valid()); // stdlib preserved
}

TEST_CASE("reset_table_to_baseline restores a nested engine table") {
    auto lua = make_state();
    lua.script("k2 = { log = function() end }"); // stands in for the bound k2 table
    sol::table k2 = lua["k2"];
    auto baseline = k2::table_string_keys(k2);

    lua.script("k2.game = { wave = 3 }"); // script hangs state off k2

    k2::reset_table_to_baseline(lua["k2"], baseline);

    REQUIRE(lua["k2"]["game"] == sol::lua_nil);
    REQUIRE(lua["k2"]["log"].valid()); // engine key kept
}
