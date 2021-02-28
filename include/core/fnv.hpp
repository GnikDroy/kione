#pragma once

#include <cstdint>

constexpr std::uint64_t operator ""_fnv(const char *str, std::size_t len) {
    std::uint64_t prime = 1099511628211;
    std::uint64_t hash = 14695981039346656037;
    for (std::size_t i = 0; i < len; i++) {
        hash *= prime;
        hash ^= str[i];
    }
    return hash;
}

