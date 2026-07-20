#include <catch2/catch_all.hpp>

#include "core/paths.hpp"

TEST_CASE("executable_path points at the running binary") {
    auto path = k2::executable_path();
    REQUIRE(path.is_absolute());
    REQUIRE(std::filesystem::is_regular_file(path));
    REQUIRE(path.filename().string().starts_with("K2_tests"));
}
