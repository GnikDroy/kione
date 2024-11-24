#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <format>
#include <numeric>

#include "core/utils.hpp"

namespace k2 {
template <arithmetic T, std::size_t N> struct Vector {
    std::array<T, N> data {};

    constexpr std::size_t size() const noexcept { return N; };

    constexpr T& operator[](const std::size_t i) noexcept { return data[i]; }
    constexpr T operator[](const std::size_t i) const noexcept { return data[i]; }

    constexpr T magnitude2() const noexcept {
        return std::accumulate(
            data.begin(), data.end(), static_cast<T>(0), [](auto left, auto right) { return left + right * right; });
    }

    constexpr double magnitude() const noexcept { return std::sqrt(magnitude2()); }

    template <arithmetic Num> constexpr auto operator+(const Vector<Num, N>& other) const noexcept {
        Vector<decltype(data[0] + other[0]), N> new_vector;
        std::transform(data.begin(), data.end(), other.data.begin(), new_vector.data.begin(), std::plus<> {});
        return new_vector;
    }

    template <arithmetic Num> constexpr auto operator+=(const Vector<Num, N>& other) noexcept {
        std::transform(data.begin(), data.end(), other.data.begin(), data.begin(), std::plus<> {});
        return *this;
    }

    template <arithmetic Num> constexpr auto operator-(const Vector<Num, N>& other) const noexcept {
        Vector<decltype(data[0] - other[0]), N> new_vector;
        std::transform(data.begin(), data.end(), other.data.begin(), new_vector.data.begin(), std::minus<> {});
        return new_vector;
    }

    template <arithmetic Num> constexpr auto operator-=(const Vector<Num, N>& other) noexcept {
        std::transform(data.begin(), data.end(), other.data.begin(), data.begin(), std::minus<> {});
        return *this;
    }

    template <arithmetic Num> constexpr auto operator*(const Vector<Num, N>& other) const noexcept {
        Vector<decltype(data[0] * other[0]), N> new_vector;
        std::transform(data.begin(), data.end(), other.data.begin(), new_vector.data.begin(), std::multiplies<> {});
        return new_vector;
    }

    template <arithmetic Num> constexpr auto operator*=(const Vector<Num, N>& other) noexcept {
        std::transform(data.begin(), data.end(), other.data.begin(), data.begin(), std::multiplies<> {});
        return *this;
    }

    template <arithmetic Num> constexpr auto operator*(const Num& c) const noexcept {
        Vector<decltype(data[0] * c), N> new_vector { *this };
        std::transform(data.begin(), data.end(), new_vector.data.begin(), [&c](auto i) { return i * c; });
        return new_vector;
    }

    template <arithmetic Num> constexpr auto operator*=(const Num& c) noexcept {
        std::transform(data.begin(), data.end(), [&c](auto i) { return i * c; });
        return *this;
    }

    template <arithmetic Num> constexpr auto operator/(const Vector<Num, N>& other) const {
        Vector<decltype(data[0] / other[0]), N> new_vector;
        std::transform(data.begin(), data.end(), other.data.begin(), new_vector.data.begin(), std::divides<> {});
        return new_vector;
    }

    template <arithmetic Num> constexpr auto operator/=(const Vector<Num, N>& other) noexcept {
        std::transform(data.begin(), data.end(), other.data.begin(), data.begin(), std::divides<> {});
        return *this;
    }

    template <arithmetic Num> constexpr auto operator/(const Num& c) const noexcept {
        Vector<decltype(data[0] / c), N> new_vector { *this };
        std::transform(data.begin(), data.end(), new_vector.data.begin(), [&c](auto i) { return i / c; });
        return new_vector;
    }

    template <arithmetic Num> constexpr auto operator/=(const Num& c) noexcept {
        std::transform(data.begin(), data.end(), [&c](auto i) { return i / c; });
        return *this;
    }

    constexpr auto operator==(const Vector& other) const noexcept { return data == other.data; }

    constexpr auto operator!=(const Vector& other) const noexcept { return data != other.data; }

    template <arithmetic Num> constexpr auto dot(const Vector<Num, N>& other) const noexcept {
        return std::inner_product(data.begin(), data.end(), other.begin(), 0);
    }
};

template <arithmetic T> struct Vector<T, 2> {
    T x {}, y {};

    constexpr Vector() = default;
    constexpr Vector(T x, T y) noexcept
        : x { x }
        , y { y } {};

    template <arithmetic Num> constexpr auto operator+(const Vector<Num, 2>& other) const noexcept {
        return Vector<decltype(x + other.x), 2> { x + other.x, y + other.y };
    }

    template <arithmetic Num> constexpr auto operator+=(const Vector<Num, 2>& other) noexcept {
        x += other.x;
        y += other.y;
        return *this;
    }

    template <arithmetic Num> constexpr auto operator-(const Vector<Num, 2>& other) const noexcept {
        return Vector<decltype(x - other.x), 2> { x - other.x, y - other.y };
    }

    template <arithmetic Num> constexpr auto operator-=(const Vector<Num, 2>& other) noexcept {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    template <arithmetic Num> constexpr auto operator*(const Vector<Num, 2>& other) const noexcept {
        return Vector<decltype(x * other.x), 2> { x * other.x, y * other.y };
    }

    template <arithmetic Num> constexpr auto operator*=(const Vector<Num, 2>& other) noexcept {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    template <arithmetic Num> constexpr auto operator*(const Num& c) const noexcept {
        return Vector<decltype(x * c), 2> { x * c, y * c };
    }

    template <arithmetic Num> constexpr auto operator*=(const Num& c) noexcept {
        x *= c;
        y *= c;
        return *this;
    }

    template <arithmetic Num> constexpr auto operator/(const Num& c) const {
        return Vector<decltype(x / c), 2> { x / c, y / c };
    }

    template <arithmetic Num> constexpr auto operator/=(const Num& c) noexcept {
        x /= c;
        y /= c;
        return *this;
    }

    template <arithmetic Num> constexpr auto dot(const Vector<Num, 2>& other) const noexcept {
        return x * other.x + y * other.y;
    }

    constexpr Vector& normalize() {
        auto mag = magnitude();
        x /= mag;
        y /= mag;
        return *this;
    }

    constexpr double magnitude() const noexcept { return std::sqrt(magnitude2()); }
    constexpr T magnitude2() const noexcept { return x * x + y * y; }

    template <arithmetic Num> constexpr bool operator==(const Vector<Num, 2>& other) const noexcept {
        return x == other.x && y == other.y;
    }

    template <arithmetic Num> constexpr bool operator!=(const Vector<Num, 2>& other) const noexcept {
        return !(*this == other);
    }

    constexpr T operator[](std::size_t i) noexcept {
        assert(i < 2);
        return i == 0 ? x : y;
    }

    constexpr std::size_t size() noexcept { return 2; }

    constexpr double angle() const noexcept { return std::atan2(y, x); }
};

template <arithmetic T> struct Vector<T, 3> {
    T x {}, y {}, z {};

    constexpr Vector() = default;
    constexpr Vector(const T& x, const T& y, const T& z) noexcept
        : x { x }
        , y { y }
        , z { z } {};

    template <arithmetic Num> constexpr auto operator+(const Vector<Num, 3>& other) const noexcept {
        return Vector<decltype(x + other.x), 3> { x + other.x, y + other.y, z + other.z };
    }

    template <arithmetic Num> constexpr auto operator+=(const Vector<Num, 3>& other) noexcept {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    template <arithmetic Num> constexpr auto operator-(const Vector<Num, 3>& other) const noexcept {
        return Vector<decltype(x - other.x), 3> { x - other.x, y - other.y, z - other.z };
    }

    template <arithmetic Num> constexpr auto operator-=(const Vector<Num, 3>& other) noexcept {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    template <arithmetic Num> constexpr auto operator/(const Num& c) const {
        return Vector<decltype(x / c), 3> { x / c, y / c, z / c };
    }

    template <arithmetic Num> constexpr auto operator/=(const Num& c) noexcept {
        x /= c;
        y /= c;
        z /= c;
        return *this;
    }

    template <arithmetic Num> constexpr auto dot(const Vector<Num, 3>& other) const noexcept {
        return x * other.x + y * other.y + z * other.z;
    }

    template <arithmetic Num> constexpr auto operator*(const Vector<Num, 3>& other) const noexcept {
        return Vector<decltype(x * other.x), 3> { x * other.x, y * other.y, z * other.z };
    }

    template <arithmetic Num> constexpr auto operator*=(const Vector<Num, 2>& other) noexcept {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    template <arithmetic Num> constexpr auto operator*(const Num& c) const noexcept {
        return Vector<decltype(x * c), 3> { x * c, y * c, z * c };
    }

    template <arithmetic Num> constexpr auto operator+=(const Num& c) noexcept {
        x *= c;
        y *= c;
        z *= c;
        return *this;
    }
    constexpr Vector& normalize() {
        auto mag = magnitude();
        x /= mag;
        y /= mag;
        z /= mag;
        return *this;
    }

    constexpr double magnitude() const noexcept { return std::sqrt(magnitude2()); }
    constexpr T magnitude2() const noexcept { return x * x + y * y + z * z; }

    template <arithmetic Num> constexpr bool operator==(const Vector<Num, 3>& other) const noexcept {
        return x == other.x && y == other.y && z == other.z;
    }

    template <arithmetic Num> constexpr bool operator!=(const Vector<Num, 3>& other) const noexcept {
        return !(*this == other);
    }

    template <arithmetic Num> constexpr T operator[](std::size_t i) noexcept {
        assert(i < 3);
        return i == 0 ? x : (i == 1 ? y : z);
    }

    constexpr std::size_t size() noexcept { return 3; }

    constexpr std::tuple<double, double, double> angle() const noexcept {
        auto ax = std::atan2(std::sqrt(y ^ 2 + z ^ 2), x);
        auto ay = std::atan2(std::sqrt(z ^ 2 + x ^ 2), y);
        auto az = std::atan2(std::sqrt(x ^ 2 + y ^ 2), z);
        return { ax, ay, az };
    }
};

template <arithmetic T> using Vector2 = Vector<T, 2>;

template <arithmetic T> using Vector3 = Vector<T, 3>;

} // namespace k2

template <class T, std::size_t N> struct std::formatter<k2::Vector<T, N>> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const k2::Vector<T, N>& vec, std::format_context& ctx) const {
        if constexpr (N == 2) {
            return format_to(ctx.out(), "[{}, {}]", vec.x, vec.y);
        } else if constexpr (N == 3) {
            return format_to(ctx.out(), "[{}, {}, {}]", vec.x, vec.y, vec.z);
        } else {
            return format_to(ctx.out(), "{}", vec.data);
        }
    }
};

template <k2::arithmetic T, std::size_t N> std::ostream& operator<<(std::ostream& out, const k2::Vector<T, N>& vec) {
    return out << std::format("{}", vec);
}
