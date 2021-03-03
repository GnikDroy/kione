#pragma once

#include <cstdint>

namespace k2::literals {
    constexpr std::uint64_t operator ""_fnv1a(const char *str, std::size_t len) {
        std::uint64_t prime = 1099511628211;
        std::uint64_t hash = 14695981039346656037ull;
        for (std::size_t i = 0; i < len; i++) {
            hash ^= str[i];
            hash *= prime;
        }
        return hash;
    }
}

