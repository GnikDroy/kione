#include <catch2/catch_all.hpp>

#include "core/vector.hpp"

TEST_CASE("Vector2 Functionality") {
    k2::Vector2<int> vec;
    REQUIRE(vec.x == 0);
    REQUIRE(vec.y == 0);
    REQUIRE(vec.magnitude2() == vec.magnitude());
    REQUIRE(vec.magnitude2() == 0);
    REQUIRE(vec.size() == 2);
    REQUIRE(vec.angle() == atan2(0, 0)); // atan2(0,0) will return some value, it
                                         // is not defined what it will return

    vec = { 2, -2 };
    k2::Vector2<int> vec2 { 1, 1 };
    REQUIRE(vec + vec2 == k2::Vector2<int> { 3, -1 });
    REQUIRE(vec - vec2 == k2::Vector2<int> { 1, -3 });
    REQUIRE(vec * vec2 == k2::Vector2<int> { 2, -2 });
    REQUIRE(vec * 2 == k2::Vector2<int>(4, -4));
    REQUIRE(vec / 2 == k2::Vector2<int>(1, -1));
    REQUIRE(vec.dot(k2::Vector2<int> { 2, 2 }) == 0);

    static_assert(std::is_same_v<decltype(vec / 2.f), k2::Vector2<float>>);

    REQUIRE(vec2.magnitude2() == 2);
    REQUIRE(vec2.magnitude() == std::sqrt(vec2.magnitude2()));
    REQUIRE(vec2.angle() == atan2(vec2.y, vec2.x));

    REQUIRE(vec[0] == 2);
    REQUIRE(vec[1] == -2);

    // Compound assignment
    vec = { 2, -2 };
    vec += vec2;
    REQUIRE(vec == k2::Vector2<int> { 3, -1 });
    vec -= vec2;
    REQUIRE(vec == k2::Vector2<int> { 2, -2 });
    vec *= vec2;
    REQUIRE(vec == k2::Vector2<int> { 2, -2 });
    vec *= 2;
    REQUIRE(vec == k2::Vector2<int> { 4, -4 });
    vec /= 2;
    REQUIRE(vec == k2::Vector2<int> { 2, -2 });
}

TEST_CASE("Vector3 Functionality") {
    k2::Vector3<int> vec;
    REQUIRE(vec.x == 0);
    REQUIRE(vec.y == 0);
    REQUIRE(vec.z == 0);
    REQUIRE(vec.magnitude2() == vec.magnitude());
    REQUIRE(vec.magnitude2() == 0);
    REQUIRE(vec.size() == 3);

    vec = { 2, -2, 2 };
    k2::Vector3<int> vec2 { 1, 1, 1 };
    REQUIRE(vec + vec2 == k2::Vector3<int> { 3, -1, 3 });
    REQUIRE(vec - vec2 == k2::Vector3<int> { 1, -3, 1 });
    REQUIRE(vec * vec2 == k2::Vector3<int> { 2, -2, 2 });
    REQUIRE(vec * 2 == k2::Vector3<int>(4, -4, 4));
    REQUIRE(vec / 2 == k2::Vector3<int>(1, -1, 1));
    REQUIRE(vec.dot(vec2) == 2);

    static_assert(std::is_same_v<decltype(vec / 2.f), k2::Vector3<float>>);

    REQUIRE(vec2.magnitude2() == 3);
    REQUIRE(vec2.magnitude() == std::sqrt(vec2.magnitude2()));

    REQUIRE(vec[0] == 2);
    REQUIRE(vec[1] == -2);
    REQUIRE(vec[2] == 2);

    // Direction angles against each axis
    auto [ax, ay, az] = k2::Vector3<int> { 1, 0, 0 }.angle();
    REQUIRE(ax == std::atan2(0.0, 1.0));
    REQUIRE(ay == std::atan2(1.0, 0.0));
    REQUIRE(az == std::atan2(1.0, 0.0));

    // Compound assignment
    vec = { 2, -2, 2 };
    vec += vec2;
    REQUIRE(vec == k2::Vector3<int> { 3, -1, 3 });
    vec -= vec2;
    REQUIRE(vec == k2::Vector3<int> { 2, -2, 2 });
    vec *= vec2;
    REQUIRE(vec == k2::Vector3<int> { 2, -2, 2 });
    vec *= 2;
    REQUIRE(vec == k2::Vector3<int> { 4, -4, 4 });
    vec /= 2;
    REQUIRE(vec == k2::Vector3<int> { 2, -2, 2 });
}

TEST_CASE("Vector<N> functionality") {
    k2::Vector<int, 5> vec;
    REQUIRE(vec.magnitude2() == vec.magnitude());
    REQUIRE(vec.magnitude2() == 0);
    REQUIRE(vec.size() == 5);

    k2::Vector<int, 5> vec2 { .data = { 1, 2, 3, 4, 5 } };
    REQUIRE(vec2.magnitude2() == 1 + 4 + 9 + 16 + 25);
    REQUIRE(vec2.magnitude() == std::sqrt(vec2.magnitude2()));

    REQUIRE(
        vec2 + k2::Vector<int, 5> { .data = { 1, 2, 3, 4, 5 } } == k2::Vector<int, 5> { .data = { 2, 4, 6, 8, 10 } });
    REQUIRE(
        vec2 - k2::Vector<int, 5> { .data = { 1, 2, 3, 4, 5 } } == k2::Vector<int, 5> { .data = { 0, 0, 0, 0, 0 } });
    REQUIRE(
        vec2 * k2::Vector<int, 5> { .data = { 1, 2, 3, 4, 5 } } == k2::Vector<int, 5> { .data = { 1, 4, 9, 16, 25 } });
    REQUIRE(vec2 * 5 == k2::Vector<int, 5> { .data = { 5, 10, 15, 20, 25 } });
    REQUIRE(
        vec2 / k2::Vector<int, 5> { .data = { 1, 2, 3, 4, 5 } } == k2::Vector<int, 5> { .data = { 1, 1, 1, 1, 1 } });
    REQUIRE(vec2.dot(vec2) == 1 + 4 + 9 + 16 + 25);

    static_assert(std::is_same_v<decltype(vec2 * 2.5f), k2::Vector<float, 5>>);
    static_assert(std::is_same_v<decltype(vec2 / 2.0), k2::Vector<double, 5>>);

    // Compound assignment
    auto vec3 = vec2;
    vec3 += vec2;
    REQUIRE(vec3 == k2::Vector<int, 5> { .data = { 2, 4, 6, 8, 10 } });
    vec3 -= vec2;
    REQUIRE(vec3 == vec2);
    vec3 *= vec2;
    REQUIRE(vec3 == k2::Vector<int, 5> { .data = { 1, 4, 9, 16, 25 } });
    vec3 /= vec2;
    REQUIRE(vec3 == vec2);
    vec3 *= 5;
    REQUIRE(vec3 == k2::Vector<int, 5> { .data = { 5, 10, 15, 20, 25 } });
    vec3 /= 5;
    REQUIRE(vec3 == vec2);
}