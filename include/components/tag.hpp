#pragma once
#include <array>
#include <string_view>

namespace k2 {
struct TagComponent {
    constexpr static auto MAX_SIZE = 64;
    std::array<char, MAX_SIZE> tag {};
    [[nodiscard]] std::string_view str() const { return { tag.data() }; }
};
}