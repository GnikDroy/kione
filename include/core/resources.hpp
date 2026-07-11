#pragma once

#include "core/resource_manager.hpp"
#include "rendering/image.hpp"
#include "rendering/shader.hpp"
#include "rendering/texture.hpp"

namespace k2 {
/**
 * The resource manager instantiation for the engine's resource types.
 *
 * Own one inside the GL context's lifetime and pass it by reference; see
 * BasicResourceManager for the rationale.
 */
using ResourceManager = BasicResourceManager<Shader, Image, Texture2D, TextureCube>;
}
