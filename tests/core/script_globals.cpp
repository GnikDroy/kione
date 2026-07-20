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
    lua.script("kione = { log = function() end }"); // stands in for the bound kione table
    sol::table kione = lua["kione"];
    auto baseline = k2::table_string_keys(kione);

    lua.script("kione.game = { wave = 3 }"); // script hangs state off kione

    k2::reset_table_to_baseline(lua["kione"], baseline);

    REQUIRE(lua["kione"]["game"] == sol::lua_nil);
    REQUIRE(lua["kione"]["log"].valid()); // engine key kept
}
