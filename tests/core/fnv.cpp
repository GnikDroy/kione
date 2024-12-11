#include <catch2/catch_all.hpp>

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
        REQUIRE(k2::fnv1a(kione) == 0x86a6488dbe53d0f3);
        REQUIRE(k2::fnv1a(kione.data(), kione.size()) == 0x86a6488dbe53d0f3);
    }

    // Literals
    {
        using namespace k2::literals;
        REQUIRE("kione"_fnv1a == 0x86a6488dbe53d0f3);
    }
}