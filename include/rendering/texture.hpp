#pragma once

#include <array>
#include <concepts>
#include <cstdint>
#include <filesystem>
#include <span>
#include <utility>

#include <glad/glad.h>

#include "image.hpp"

namespace k2 {
template <typename T>
concept FloatOrUInt8 = std::same_as<T, float> || std::same_as<T, uint8_t>;

template <FloatOrUInt8 T> struct OpenGLType;

template <> struct OpenGLType<float> {
    static constexpr GLenum type = GL_FLOAT;
};

template <> struct OpenGLType<uint8_t> {
    static constexpr GLenum type = GL_UNSIGNED_BYTE;
};

enum class TextureFilter : std::uint8_t { Nearest, Linear };

struct Texture2D {
    template <FloatOrUInt8 T> static GLint predict_sized_format(std::size_t channels);
    static GLenum predict_format_from_sized(std::size_t sized);
    static GLenum predict_type_from_sized(std::size_t sized);

    GLuint id {};
    int width {};
    int height {};

    Texture2D() = default;

    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    Texture2D(Texture2D&& other) noexcept { *this = std::move(other); }

    Texture2D& operator=(Texture2D&& other) noexcept {
        std::swap(other.id, id);
        std::swap(other.width, width);
        std::swap(other.height, height);
        return *this;
    }

    ~Texture2D() { glDeleteTextures(1, &id); }

    explicit Texture2D(const Image& image, bool generate_mipmaps = true, TextureFilter filter = TextureFilter::Linear) {
        load(image, generate_mipmaps, filter);
    }

    template <FloatOrUInt8 T>
    Texture2D(std::size_t width, std::size_t height, std::span<const T> data = {}, GLuint sized_format = GL_RGBA8,
        bool generate_mipmaps = true, TextureFilter filter = TextureFilter::Linear);

    explicit operator bool() const { return id != 0; }

    void bind(std::uint32_t texture_unit) const {
        if (*this) {
            glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + texture_unit));
            glBindTexture(GL_TEXTURE_2D, id);
        }
    }

    template <FloatOrUInt8 T> static Texture2D create_white_texture();

    Texture2D& load(const Image& image, bool generate_mipmaps = true, TextureFilter filter = TextureFilter::Linear);
};

struct TextureCube {
    GLuint id {};
    std::array<std::filesystem::path, 6> paths;

    TextureCube() = default;

    TextureCube(const TextureCube&) = delete;
    TextureCube& operator=(const TextureCube&) = delete;

    TextureCube(TextureCube&& other) noexcept { *this = std::move(other); }

    TextureCube& operator=(TextureCube&& other) noexcept {
        std::swap(other.id, id);
        std::swap(other.paths, paths);
        return *this;
    }

    ~TextureCube() { glDeleteTextures(1, &id); }

    explicit TextureCube(const std::array<std::filesystem::path, 6>& paths)
        : paths(paths) {
        load(paths);
    }

    explicit operator bool() const { return id != 0; }

    TextureCube& load(const std::array<std::filesystem::path, 6>& texture_paths);
};
}
