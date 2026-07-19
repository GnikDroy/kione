#pragma once

#include "core/audio.hpp"
#include "core/resource_manager.hpp"
#include "rendering/font.hpp"
#include "rendering/image.hpp"
#include "rendering/sprite_animation.hpp"
#include "rendering/shader.hpp"
#include "rendering/texture.hpp"

namespace k2 {
using ResourceManager = BasicResourceManager<Shader, Image, Texture2D, TextureCube, SpriteAnimation, Font, AudioClip>;
}
