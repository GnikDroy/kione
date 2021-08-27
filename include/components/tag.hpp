#pragma once
#include <array>
#include <string_view>

namespace k2 {
struct TagComponent {
    std::array<char, 50> tag {};

    std::string_view str() const { return { tag.data() }; }
};
}