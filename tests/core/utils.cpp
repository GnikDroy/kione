#include <catch2/catch_all.hpp>

#include "core/utils.hpp"

TEST_CASE("String View Split") {
    // Empty string view
    {
        auto res = k2::string_view_split({});
        REQUIRE(k2::string_view_split({}).size() == 1);
        REQUIRE(k2::string_view_split({}, ',').size() == 1);
    }

    // One char string view
    {
        auto res = k2::string_view_split(",");
        REQUIRE(k2::string_view_split(",").size() == 1);
        REQUIRE(k2::string_view_split(",")[0] == ",");
    }

    // One element string view
    {
        auto res = k2::string_view_split("One element", ',');
        REQUIRE(res.size() == 1);
        REQUIRE(res[0] == "One element");
    }

    // Two item string view
    {
        auto res = k2::string_view_split("One element");
        REQUIRE(res.size() == 2);
        REQUIRE(res[0] == "One");
        REQUIRE(res[1] == "element");
    }

    // Multiple delim element string view
    {
        auto res = k2::string_view_split(",,,", ',');
        REQUIRE(res.size() == 4);
        for (auto& i : res) {
            REQUIRE(i.size() == 0);
        }
    }

    // Combined cases
    {
        auto res = k2::string_view_split(",a,b,c,d,,e,,", ',');
        REQUIRE(res.size() == 9);
        REQUIRE(res == std::vector<std::string_view> { "", "a", "b", "c", "d", "", "e", "", "" });
    }
}

TEST_CASE("StringView to Integer") {
    {
        std::string_view sv { "123" };
        REQUIRE(k2::to_number<std::uint32_t>(sv.data(), sv.data() + sv.size()) == 123);
    }

    {
        std::string_view sv { "" };
        REQUIRE_THROWS_AS(k2::to_number<std::uint32_t>(sv.data(), sv.data() + sv.size()), std::invalid_argument);
    }

    {
        std::string_view sv { "abc" };
        REQUIRE_THROWS_AS(k2::to_number<std::uint32_t>(sv.data(), sv.data() + sv.size()), std::invalid_argument);
    }

    {
        // from_chars rejects the sign for unsigned targets.
        std::string_view sv { "-1" };
        REQUIRE_THROWS_AS(k2::to_number<std::uint32_t>(sv.data(), sv.data() + sv.size()), std::invalid_argument);
    }

    {
        std::string_view sv { "12.12" };
        REQUIRE(k2::to_number<std::uint32_t>(sv.data(), sv.data() + sv.size()) == 12);
    }

    {
        std::string_view sv { "12.12" };
        REQUIRE(k2::to_number<float>(sv.data(), sv.data() + sv.size()) == 12.12f);
    }
}
