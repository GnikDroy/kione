#pragma once
#include "core/fnv.hpp"

namespace k2 {
struct Event {
    std::uint64_t type;
    explicit Event(std::uint64_t type);
    virtual ~Event() = default;
};
}