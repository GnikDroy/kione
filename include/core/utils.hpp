#pragma once
#include <charconv>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace k2 {
template <class T, class U> using result = std::pair<T, U>;

template <class T>
concept arithmetic = std::is_arithmetic_v<T>;

template <arithmetic T> struct Rect {
    T x {};
    T y {};
    T w {};
    T h {};
};

using Rectf = Rect<float>;
using Recti = Rect<int>;

template <class...> constexpr std::false_type always_false {};

// Works similar to the python split() function.
// No inconsistency between ' ' and other characters (unlike in python)
// splitting with ' ' is similar to splitting with any other characters (unlike in python).
inline std::vector<std::string_view> string_view_split(const std::string_view& str, char deliminator = ' ') {
    std::vector<std::string_view> vec;
    std::size_t start {}, next;
    do {
        next = std::min(str.find(deliminator, start), str.size());
        vec.emplace_back(str.data() + start, next - start);
        start = next + 1;
    } while (next != str.size());
    return vec;
}

template <arithmetic T, class... Args> T to_integer(const char* first, const char* last, Args... args) {
    T t;
    std::from_chars_result res = std::from_chars(first, last, t, args...);
    if (res.ec == std::errc::invalid_argument) {
        throw std::invalid_argument { "invalid_argument while converting string view to integer." };
    } else if (res.ec == std::errc::result_out_of_range) {
        throw std::out_of_range { "out_of_range while converting string view to integer." };
    }
    return t;
}
}