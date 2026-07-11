/**
 * @file resource_container.hpp
 * @author Gnik Droy
 * @brief File containing functions/classes for managing resources.
 *
 */
#pragma once
#include <unordered_map>

namespace k2 {
/**
 * @class ResourceContainer
 * @brief Class for managing resources.
 */
using ResourceID = uint64_t;

template <class ResourceType> class ResourceContainer {
public:
    /**
     * @brief Returns the number of resources currently held by the game.
     * @return Total number of resources held.
     */
    [[nodiscard]] std::size_t size() const { return resources.size(); }

    [[nodiscard]] bool contains(ResourceID resource_id) const { return resources.count(resource_id); }

    ResourceType& operator[](ResourceID id) { return resources.at(id); }

    const ResourceType& operator[](ResourceID id) const { return resources.at(id); }

    ResourceType& insert_or_assign(ResourceID id, ResourceType value) {
        return resources.insert_or_assign(id, std::move(value)).first->second;
    }

    void erase(ResourceID id) { resources.erase(id); }

    void clear() { resources.clear(); }

    auto begin() const { return resources.cbegin(); }
    auto end() const { return resources.cend(); }

    auto begin() { return resources.begin(); }
    auto end() { return resources.end(); }

private:
    /** Container that stores the identifier and Resource mappings.*/
    std::unordered_map<ResourceID, ResourceType> resources;
};
} // namespace k2
