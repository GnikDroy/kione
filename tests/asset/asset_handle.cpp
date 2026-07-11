#include <catch2/catch_all.hpp>

#include "serializers/asset/asset_handle.hpp"

using namespace k2::literals;

TEST_CASE("AssetHandle invariants") {
    k2::AssetHandle handle { "icon_file" };
    REQUIRE(handle.name == "icon_file");
    REQUIRE(handle.id == "icon_file"_fnv1a);

    handle.set("tex");
    REQUIRE(handle.name == "tex");
    REQUIRE(handle.id == "tex"_fnv1a);

    REQUIRE(k2::AssetHandle {}.id == ""_fnv1a);
    REQUIRE(k2::AssetHandle {} == k2::AssetHandle { "" });

    REQUIRE(k2::AssetHandle { "tex" } == k2::AssetHandle { "tex" });
    REQUIRE(!(k2::AssetHandle { "tex" } == k2::AssetHandle { "white" }));
}

TEST_CASE("AssetHandle YAML round trip") {
    {
        auto node = YAML::Node(k2::AssetHandle { "icon_file" });
        REQUIRE(node.IsScalar());
        REQUIRE(node.as<std::string>() == "icon_file");

        auto decoded = node.as<k2::AssetHandle>();
        REQUIRE(decoded.name == "icon_file");
        REQUIRE(decoded.id == "icon_file"_fnv1a);
    }

    {
        auto decoded = YAML::Node(k2::AssetHandle {}).as<k2::AssetHandle>();
        REQUIRE(decoded == k2::AssetHandle {});
        REQUIRE(decoded.name.empty());
    }
}
