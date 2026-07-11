#include <catch2/catch_all.hpp>

#include "core/fnv.hpp"
#include "core/resource_container.hpp"

#include <stdexcept>

TEST_CASE("Resource Container tests") {
    using k2::literals::operator""_fnv1a;

    k2::ResourceContainer<int> textures;
    REQUIRE(textures.size() == 0);
    REQUIRE(!textures.contains("test_texture"_fnv1a));
    REQUIRE(textures.begin() == textures.end());

    REQUIRE_THROWS_AS(textures["test_texture"_fnv1a], std::out_of_range);

    textures.insert_or_assign("test_texture"_fnv1a, 5);
    REQUIRE(textures.size() == 1);
    REQUIRE(textures["test_texture"_fnv1a] == 5);
    REQUIRE(textures.contains("test_texture"_fnv1a));
    REQUIRE(textures.begin()->first == "test_texture"_fnv1a);
    REQUIRE(textures.begin()->second == textures["test_texture"_fnv1a]);

    textures.insert_or_assign("test_texture"_fnv1a, 7);
    REQUIRE(textures.size() == 1);
    REQUIRE(textures["test_texture"_fnv1a] == 7);

    textures.erase("test_texture"_fnv1a);
    REQUIRE(textures.size() == 0);
}
