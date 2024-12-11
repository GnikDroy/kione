#pragma once

#include <tuple>

#include "core/resource_container.hpp"
#include "rendering/image.hpp"
#include "rendering/shader.hpp"
#include "rendering/texture.hpp"

namespace k2 {
template <class... Args> class BasicResources {
    std::tuple<ResourceContainer<Args>...> resources {};

    BasicResources() = default;

public:
    static BasicResources& get() {
        static BasicResources resources;
        return resources;
    }

    template <class T> static ResourceContainer<T>& get() { return std::get<ResourceContainer<T>>(get().resources); }

    template <class T> static T& get(ResourceID id) { return get<T>()[id]; }
    template <class T> static T* try_get(ResourceID id) { return get<T>().contains(id) ? &get<T>(id) : nullptr; }
};

using Resources = BasicResources<Shader, Image, Texture2D, TextureCube>;
}
