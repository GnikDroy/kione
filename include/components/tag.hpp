#pragma once
#include <array>
#include <string_view>

namespace k2 {
struct TagComponent {
    std::array<char, 50> tag {};
    [[nodiscard]] std::string_view str() const { return { tag.data() }; }
};
}