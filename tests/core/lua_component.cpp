#include <catch2/catch_all.hpp>

#include "core/script/lua_component.hpp"

TEST_CASE("lua_component creates one table per entity and reuses it") {
    sol::state lua;
    entt::registry registry;
    auto first = registry.create();
    auto second = registry.create();

    auto& table = k2::lua_component(lua, registry, first);
    table["health"] = lua.create_table_with("hp", 10);

    REQUIRE(k2::lua_component(lua, registry, first)["health"]["hp"].get<int>() == 10);
    REQUIRE_FALSE(k2::lua_component(lua, registry, second)["health"].valid()); // distinct tables
    REQUIRE(registry.all_of<k2::LuaComponent>(first));
}

TEST_CASE("deep_copy_table copies nested tables and keeps them independent") {
    sol::state lua;
    lua.open_libraries(sol::lib::base);
    lua.script("source = { kind = 'grunt', stats = { hp = 5, speed = 90 } }");
    sol::table source = lua["source"];

    auto copy = k2::deep_copy_table(source);
    copy["stats"]["hp"] = 1;
    copy["kind"] = "tank";

    REQUIRE(source["stats"]["hp"].get<int>() == 5);
    REQUIRE(source["kind"].get<std::string>() == "grunt");
    REQUIRE(copy["stats"]["speed"].get<int>() == 90);
}

TEST_CASE("deep_copy_table survives cycles and shares functions") {
    sol::state lua;
    lua.open_libraries(sol::lib::base);
    lua.script("source = { fn = function() return 7 end }; source.self = source");
    sol::table source = lua["source"];

    auto copy = k2::deep_copy_table(source);

    sol::table copied_self = copy["self"];
    REQUIRE(copied_self.pointer() == copy.pointer()); // cycle maps onto the copy
    REQUIRE(copied_self.pointer() != source.pointer());
    sol::function copied_fn = copy["fn"];
    sol::function source_fn = source["fn"];
    REQUIRE(copied_fn.pointer() == source_fn.pointer()); // functions are shared by reference
    REQUIRE(copied_fn().get<int>() == 7);
}

TEST_CASE("the lua component dies with the entity") {
    sol::state lua;
    entt::registry registry;
    auto entity = registry.create();
    k2::lua_component(lua, registry, entity)["gold"] = 5;

    registry.destroy(entity); // releases the component and its table reference

    auto fresh = registry.create(); // recycled slot gets a fresh table
    REQUIRE_FALSE(k2::lua_component(lua, registry, fresh)["gold"].valid());
}
