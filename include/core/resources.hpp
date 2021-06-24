#pragma once

#include <optional>
#include <tuple>

#include "core/resource_container.hpp"

#include "core/rendering/image.hpp"
#include "core/rendering/shader.hpp"
#include "core/rendering/texture.hpp"

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
};

using Resources = BasicResources<Shader, Image, Texture2D, TextureCube>;
}