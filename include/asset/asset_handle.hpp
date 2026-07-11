#pragma once

#include <string>
#include <utility>

#include "core/fnv.hpp"
#include "core/resource_container.hpp"

namespace k2 {
// Invariant: id == fnv1a(name); write through set(), never the fields.
struct AssetHandle {
    std::string name {};
    ResourceID id { fnv1a(name) };

    AssetHandle() = default;

    explicit AssetHandle(std::string name)
        : name { std::move(name) }
        , id { fnv1a(this->name) } { }

    void set(std::string new_name) {
        name = std::move(new_name);
        id = fnv1a(name);
    }

    bool operator==(const AssetHandle& other) const { return id == other.id; }
};
}
