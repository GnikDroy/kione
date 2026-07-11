#pragma once

#include "core/resource_manager.hpp"
#include "rendering/image.hpp"
#include "rendering/shader.hpp"
#include "rendering/texture.hpp"

namespace k2 {
using ResourceManager = BasicResourceManager<Shader, Image, Texture2D, TextureCube>;
}
