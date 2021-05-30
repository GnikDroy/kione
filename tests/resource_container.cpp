#include "catch2/catch.hpp"
#include "core/resource_container.hpp"
#include "core/fnv.hpp"

TEST_CASE("Resource Manager tests")
{
  using k2::literals::operator""_fnv1a;
    {
        // Mocking textures with ints.
        k2::ResourceContainer<int> textures;
        REQUIRE(textures.size() == 0);
        REQUIRE(!textures.contains("test_texture"_fnv1a));
        REQUIRE(textures.begin() == textures.end());
        REQUIRE(textures["test_texture"_fnv1a] == 0);
        REQUIRE(textures.size() == 1);

        textures["test_texture"_fnv1a] = 5;
        REQUIRE(textures.size() == 1);
        REQUIRE(textures["test_texture"_fnv1a] == 5);
        REQUIRE(textures.contains("test_texture"_fnv1a));
        REQUIRE(textures.begin()->first == "test_texture"_fnv1a);
        REQUIRE(textures.begin()->second == textures["test_texture"_fnv1a]);

        textures.erase("test_texture"_fnv1a);
        REQUIRE(textures.size() == 0);
    }

    {
        // Mocking textures with const ints.
        k2::ResourceContainer<const int> textures;
        REQUIRE(textures.size() == 0);
        REQUIRE(!textures.contains("test_texture"_fnv1a));
        REQUIRE(textures.begin() == textures.end());
        REQUIRE(textures["test_texture"_fnv1a] == 0);
        REQUIRE(textures.size() == 1);

        textures["test_texture"_fnv1a];
        REQUIRE(textures.size() == 1);
        REQUIRE(textures.contains("test_texture"_fnv1a));
        REQUIRE(textures.begin()->first == "test_texture"_fnv1a);
        REQUIRE(textures.begin()->second == textures["test_texture"_fnv1a]);

        textures.erase("test_texture"_fnv1a);
        REQUIRE(textures.size() == 0);
    }
}

