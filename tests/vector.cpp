#include "catch2/catch.hpp"
#include "core/vector.hpp"

TEST_CASE("Vector2 Functionality")
{
  k2::Vector2<int> vec;
  REQUIRE(vec.x == 0);
  REQUIRE(vec.y == 0); 
  REQUIRE(vec.magnitude2() == vec.magnitude());
  REQUIRE(vec.magnitude2() == 0);
  REQUIRE(vec.size() == 2);
  REQUIRE(vec.angle() == atan2(0, 0)); // atan2(0,0) will return some value, it
                                       // is not defined what it will return

  vec = {2, -2};
  k2::Vector2<int> vec2{1, 1};
  REQUIRE(vec + vec2 == k2::Vector2<int>{3, -1});
  REQUIRE(vec - vec2 == k2::Vector2<int>{1, -3});
  REQUIRE(vec * vec2 == k2::Vector2<int>{2, -2});
  REQUIRE(vec * 2 == k2::Vector2<int>(4, -4));
  REQUIRE(vec / 2 == k2::Vector2<int>(1, -1));
  REQUIRE(vec.dot(k2::Vector2<int>{2, 2}) == 0);

  auto x = vec / 2.f;
  static_assert(std::is_same_v<decltype(x), k2::Vector2<float>>);

  REQUIRE(vec2.magnitude2() == 2);
  REQUIRE(vec2.magnitude() == std::sqrt(vec2.magnitude2()));
  REQUIRE(vec2.angle() == atan2(vec2.y, vec2.x));
}

TEST_CASE("Vector3 Functionality")
{
  k2::Vector3<int> vec;
  REQUIRE(vec.x == 0);
  REQUIRE(vec.y == 0);
  REQUIRE(vec.z == 0);
  REQUIRE(vec.magnitude2() == vec.magnitude());
  REQUIRE(vec.magnitude2() == 0);
  REQUIRE(vec.size() == 3);

  vec = {2, -2, 2};
  k2::Vector3<int> vec2{1, 1, 1};
  REQUIRE(vec + vec2 == k2::Vector3<int>{3, -1, 3});
  REQUIRE(vec - vec2 == k2::Vector3<int>{1, -3, 1});
  REQUIRE(vec * vec2 == k2::Vector3<int>{2, -2, 2});
  REQUIRE(vec * 2 == k2::Vector3<int>(4, -4, 4));
  REQUIRE(vec / 2 == k2::Vector3<int>(1, -1, 1));
  REQUIRE(vec.dot(vec2) == 2);

  auto x = vec / 2.f;
  static_assert(std::is_same_v<decltype(x), k2::Vector3<float>>);

  REQUIRE(vec2.magnitude2() == 3);
  REQUIRE(vec2.magnitude() == std::sqrt(vec2.magnitude2()));
  // REQUIRE(vec.angle() == atan2(vec.y, vec.x));
}

TEST_CASE("Vector<N> functionality")
{
  k2::Vector<int, 5> vec;
  REQUIRE(vec.magnitude2() == vec.magnitude());
  REQUIRE(vec.magnitude2() == 0);
  REQUIRE(vec.size() == 5);

  k2::Vector<int, 5> vec2{.data = {1, 2, 3, 4, 5}};
  REQUIRE(vec2.magnitude2() == 1 + 4 + 9 + 16 + 25);
  REQUIRE(vec2.magnitude() == std::sqrt(vec2.magnitude2()));

  REQUIRE(vec2 + k2::Vector<int, 5>{.data = {1, 2, 3, 4, 5}} == k2::Vector<int, 5>{.data = {2, 4, 6, 8, 10}});
  REQUIRE(vec2 - k2::Vector<int, 5>{.data = {1, 2, 3, 4, 5}} == k2::Vector<int, 5>{.data = {0, 0, 0, 0, 0}});
  REQUIRE(vec2 * k2::Vector<int, 5>{.data = {1, 2, 3, 4, 5}} == k2::Vector<int, 5>{.data = {1, 4, 9, 16, 25}});
  REQUIRE( vec2 * 5== k2::Vector<int, 5>{.data = {5, 10, 15, 20, 25}});
  REQUIRE(vec2 / k2::Vector<int, 5>{.data = {1, 2, 3, 4, 5}} == k2::Vector<int, 5>{.data = {1,1,1,1,1}});
}