#pragma once

#include "core/audio_clip.hpp"
#include "core/resource_manager.hpp"
#include "core/script/script.hpp"
#include "rendering/font.hpp"
#include "rendering/image.hpp"
#include "rendering/sprite_animation.hpp"
#include "rendering/shader.hpp"
#include "rendering/texture.hpp"

namespace k2 {
using ResourceManager
    = BasicResourceManager<Shader, Image, Texture2D, TextureCube, SpriteAnimation, Font, AudioClip, Script>;
}
