#include <catch2/catch.hpp>

#include "core/fnv.hpp"

TEST_CASE("FNV hash works") {
    // String hash empty
    {
        const std::string empty { "" };
        REQUIRE(k2::fnv1a(empty.data(), empty.size()) == 14695981039346656037ull);
    }

    // String hash
    {
        const std::string kione { "kione" };
        REQUIRE(k2::fnv1a(kione) == 0xcbf29ce484222325);
        REQUIRE(k2::fnv1a(kione.data(), kione.size()) == 0xcbf29ce484222325);
    }

    // Literals
    {
        using namespace k2::literals;
        REQUIRE("kione"_fnv1a == 0xcbf29ce484222325);
    }
}