#include <catch2/catch_all.hpp>

#include "core/resource_manager.hpp"

#include <string>

TEST_CASE("ResourceManager functionality") {
    k2::BasicResourceManager<int, std::string> manager;
    REQUIRE(manager.all<int>().size() == 0);

    // Named registration and lookup
    auto& value = manager.set("answer", 42);
    REQUIRE(value == 42);
    REQUIRE(manager.get<int>("answer") == 42);
    REQUIRE(manager.get<int>(k2::fnv1a("answer")) == 42);
    REQUIRE(manager.contains<int>(k2::fnv1a("answer")));
    REQUIRE(manager.try_get<int>("answer") != nullptr);
    REQUIRE(manager.try_get<int>("missing") == nullptr);

    // resolve() matches the fnv1a literal used across the engine
    using namespace k2::literals;
    REQUIRE(decltype(manager)::resolve("answer") == "answer"_fnv1a);

    // Reverse lookup
    REQUIRE(manager.name_of("answer"_fnv1a) != nullptr);
    REQUIRE(*manager.name_of("answer"_fnv1a) == "answer");
    REQUIRE(manager.name_of("missing"_fnv1a) == nullptr);

    // Same name in a different type container is a distinct resource
    manager.set("answer", std::string { "forty-two" });
    REQUIRE(manager.get<std::string>("answer") == "forty-two");
    REQUIRE(manager.get<int>("answer") == 42);

    // Raw-id registration carries no name
    manager.set(k2::ResourceID { 7 }, 21);
    REQUIRE(manager.get<int>(k2::ResourceID { 7 }) == 21);
    REQUIRE(manager.name_of(k2::ResourceID { 7 }) == nullptr);

    manager.clear();
    REQUIRE(manager.all<int>().size() == 0);
    REQUIRE(manager.all<std::string>().size() == 0);
    REQUIRE(manager.name_of("answer"_fnv1a) == nullptr);
}
