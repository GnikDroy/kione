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

    // Bytes >= 0x80 must hash as octets, not sign-extended chars
    {
        using namespace k2::literals;
        REQUIRE("\xff"_fnv1a == 0xaf64724c8602eb6e);
        REQUIRE("n\xc3\xa9"_fnv1a == 0x2352a319273b5ced); // "né" in UTF-8
    }
}