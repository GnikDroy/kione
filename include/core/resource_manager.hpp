#pragma once

#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "core/fnv.hpp"
#include "core/resource_container.hpp"

namespace k2 {

/**
 * @class BasicResourceManager
 * @brief Owns resources of the given types along with the name <-> id mapping.
 *
 * Ids are FNV-1a hashes of names. Registering through set(name, ...) records the
 * name, so name_of() can answer "which asset is this id?" — required to re-resolve
 * resources referenced by serialized scenes.
 */
template <class... Args> class BasicResourceManager {
    std::tuple<ResourceContainer<Args>...> resources {};
    std::unordered_map<ResourceID, std::string> names {};

public:
    BasicResourceManager() = default;
    BasicResourceManager(const BasicResourceManager&) = delete;
    BasicResourceManager& operator=(const BasicResourceManager&) = delete;
    BasicResourceManager(BasicResourceManager&&) noexcept = default;
    BasicResourceManager& operator=(BasicResourceManager&&) noexcept = default;

    static constexpr ResourceID resolve(std::string_view name) { return fnv1a(name.data(), name.size()); }

    template <class T> ResourceContainer<T>& all() { return std::get<ResourceContainer<T>>(resources); }
    template <class T> const ResourceContainer<T>& all() const { return std::get<ResourceContainer<T>>(resources); }

    template <class T> T& set(std::string name, T value) {
        auto id = resolve(name);
        names.insert_or_assign(id, std::move(name));
        return all<T>()[id] = std::move(value);
    }

    template <class T> T& set(ResourceID id, T value) { return all<T>()[id] = std::move(value); }

    template <class T> T& get(ResourceID id) { return all<T>()[id]; }
    template <class T> T& get(std::string_view name) { return get<T>(resolve(name)); }

    template <class T> T* try_get(ResourceID id) {
        auto& container = all<T>();
        return container.contains(id) ? &container[id] : nullptr;
    }
    template <class T> T* try_get(std::string_view name) { return try_get<T>(resolve(name)); }

    template <class T> [[nodiscard]] bool contains(ResourceID id) const { return all<T>().contains(id); }

    [[nodiscard]] const std::string* name_of(ResourceID id) const {
        auto it = names.find(id);
        return it != names.end() ? &it->second : nullptr;
    }

    void clear() {
        (all<Args>().clear(), ...);
        names.clear();
    }
};
}
