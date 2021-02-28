#include "catch2/catch.hpp"
#include "core/resource_container.hpp"
#include "core/fnv.hpp"

TEST_CASE("Resource Manager tests")
{

  // Mocking textures with ints.
  k2::ResourceContainer<int> textures;
  REQUIRE(textures.size() == 0);
  REQUIRE(!textures.is_loaded("test_texture"_fnv));
  REQUIRE(textures.begin() == textures.end());
  REQUIRE(textures["test_texture"_fnv] == 0);
  REQUIRE(textures.size() == 1);

  textures["test_texture"_fnv] = 5;
  REQUIRE(textures.size() == 1);
  REQUIRE(textures["test_texture"_fnv] == 5);
  REQUIRE(textures.is_loaded("test_texture"_fnv));
  REQUIRE(textures.begin()->first == "test_texture"_fnv);
  REQUIRE(textures.begin()->second == textures["test_texture"_fnv]);

  textures.erase("test_texture"_fnv);
  REQUIRE(textures.size() == 0);
}
