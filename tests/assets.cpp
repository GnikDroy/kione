#include <catch2/catch.hpp>

#include "asset/asset.hpp"
#include <format>
#include <string>
#include <vector>

TEST_CASE("Asset URL split") {
    std::vector<std::pair<std::string, k2::Asset::URL>> url_splits {
        {
            "file://",
            k2::Asset::URL {
                .scheme { "file" },
                .authority {},
                .path {},
                .query {},
                .fragment {},
            },
        },
        {
            "file://authority/this/is/the/path?query=true#fragment",
            k2::Asset::URL {
                .scheme { "file" },
                .authority { "authority" },
                .path { "this/is/the/path" },
                .query { "query=true" },
                .fragment { "fragment" },
            },
        },
        {
            "file:////this/is/absolute/path?query=true&1=1#fragment",
            k2::Asset::URL {
                .scheme { "file" },
                .authority {},
                .path { "/this/is/absolute/path" },
                .query { "query=true&1=1" },
                .fragment { "fragment" },
            },
        },
        {
            "file:///C:/this/is/absolute/path\\with\\different\\slashes?query=true&1=1#fragment",
            k2::Asset::URL {
                .scheme { "file" },
                .authority {},
                .path { "C:/this/is/absolute/path\\with\\different\\slashes" },
                .query { "query=true&1=1" },
                .fragment { "fragment" },
            },
        },
        {
            "https:///this/is/the/path#with_only_fragment",
            k2::Asset::URL {
                .scheme { "https" },
                .authority {},
                .path { "this/is/the/path" },
                .query {},
                .fragment { "with_only_fragment" },
            },
        },
        {
            {},
            k2::Asset::URL {},
        },
    };
    for (auto& [url, splits] : url_splits) {
        REQUIRE(k2::Asset { .url { url } }.get_url_divisions() == splits);
    }
}

TEST_CASE("Asset query pair extraction.") {
    using QueryMap = std::unordered_map<std::string_view, std::string_view>;
    std::vector<std::pair<std::string, QueryMap>> cases {
        {
            {},
            {},
        },
        {
            "file:///p?a=b&c=d",
            QueryMap {
                { "a", "b" },
                { "c", "d" },
            },
        },
        {
            "file:///p?a=b",
            QueryMap {
                { "a", "b" },
            },
        },
        {
            "https://www.example.com/index.html?a",
            QueryMap {
                { "a", "" },
            },
        },
        {
            "https://www.example.com/index.html?a&b",
            QueryMap {
                { "a", "" },
                { "b", "" },
            },
        },
        {
            "https://www.example.com/index.html?=b",
            QueryMap {
                { "", "b" },
            },
        },
        { "https://www.example.com/index.html?=&=",
            QueryMap {
                { "", "" },
            } },
        { "https://www.example.com/index.html?&&&",
            QueryMap {
                { "", "" },
            } },
    };

    for (auto& [url, map] : cases) {
        INFO(std::format("{}", k2::Asset { .url = url }.get_traits()));
        INFO(std::format("{}", map));
        REQUIRE(k2::Asset { .url = url }.get_traits() == map);
    }
}
