#pragma once

#include <concepts>
#include <format>
#include <glad/glad.h>
#include <span>

#include "core/logger.hpp"
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

struct Texture2D {
    template <FloatOrUInt8 T> static auto predict_sized_format(std::size_t channels) {
        if constexpr (std::same_as<T, uint8_t>) {
            switch (channels) {
            case 1: return GL_R8;
            case 3: return GL_RGB8;
            case 4: return GL_RGBA8;
            }
        } else if constexpr (std::same_as<T, float>) {
            switch (channels) {
            case 1: return GL_R32F;
            case 3: return GL_RGB32F;
            case 4: return GL_RGBA32F;
            }
        }

        k2::Log::core().warn(std::format("Incorrect number of channels ({}) for image, assuming RGBA8.", channels));
        return GL_RGBA8;
    }

    static auto predict_format_from_sized(std::size_t sized) {
        switch (sized) {
        case GL_R8:
        case GL_R32F: return GL_RED;

        case GL_RGB8:
        case GL_RGB32F: return GL_RGB;

        case GL_RGBA8:
        case GL_RGBA32F: return GL_RGBA;

        case GL_DEPTH24_STENCIL8: return GL_DEPTH_STENCIL;
        case GL_DEPTH_COMPONENT: return GL_DEPTH_COMPONENT;
        case GL_STENCIL_INDEX8: return GL_STENCIL_INDEX;
        default: throw std::invalid_argument("Unsupported sized format received.");
        }
    }

    GLuint id {};

    Texture2D() = default;

    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    Texture2D(Texture2D&& other) noexcept { *this = std::move(other); }

    Texture2D& operator=(Texture2D&& other) noexcept {
        std::swap(other.id, id);
        return *this;
    }

    ~Texture2D() { glDeleteTextures(1, &id); }

    explicit Texture2D(const Image& image, bool generate_mipmaps = true) { load(image, generate_mipmaps); }

    template <FloatOrUInt8 T>
    Texture2D(std::size_t width, std::size_t height, std::span<const T> data = {}, GLuint sized_format = GL_RGBA8,
        bool generate_mipmaps = true) {
        glGenTextures(1, &id);
        auto levels = (GLsizei)(std::floor(std::log2(std::max(width, height))) + 1);
        if (!generate_mipmaps) {
            levels = 1;
        }
        glBindTexture(GL_TEXTURE_2D, id);
        glTexStorage2D(GL_TEXTURE_2D, levels, sized_format, (GLsizei)width, (GLsizei)height);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (!data.empty()) {
            if (width * height > data.size()) {
                throw std::invalid_argument("Insufficient data provided.");
            }
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (GLsizei)width, (GLsizei)height,
                predict_format_from_sized(sized_format), OpenGLType<T>::type, data.data());
            if (generate_mipmaps) {
                glGenerateMipmap(GL_TEXTURE_2D);
            }
        }
    }

    explicit operator bool() const { return id != 0; }

    void bind(std::uint32_t texture_unit) const {
        if (*this) {
            glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + texture_unit));
            glBindTexture(GL_TEXTURE_2D, id);
        }
    }

    template <FloatOrUInt8 T> static Texture2D create_white_texture() {
        Texture2D texture;
        glGenTextures(1, &texture.id);
        glBindTexture(GL_TEXTURE_2D, texture.id);

        if constexpr (std::same_as<T, uint8_t>) {
            std::array<T, 4> data = { 255, 255, 255, 255 };
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA, 1, 1);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA8, GL_UNSIGNED_BYTE, data.data());
        } else if constexpr (std::same_as<T, float>) {
            std::array<T, 4> data = { 1.0f, 1.0f, 1.0f, 1.0f };
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA, 1, 1);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA32F, GL_FLOAT, data.data());
        }
        return texture;
    }

    Texture2D& load(const Image& image, bool generate_mipmaps = true) {
        if (id == 0) {
            if (image) {
                glGenTextures(1, &id);
                glBindTexture(GL_TEXTURE_2D, id);

                auto levels = (GLsizei)(std::floor(std::log2(std::max(image.width, image.height))) + 1);
                if (!generate_mipmaps) {
                    levels = 1;
                }

                std::visit(
                    [&](auto& image_data) {
                        auto ptr = image_data.get();
                        if constexpr (std::is_same_v<decltype(ptr), std::uint8_t*>) {
                            auto sized_format = predict_sized_format<std::uint8_t>(image.channels);
                            auto format = predict_format_from_sized(sized_format);

                            glTexStorage2D(GL_TEXTURE_2D, levels, sized_format, image.width, image.height);
                            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width, image.height, format,
                                OpenGLType<uint8_t>::type, ptr);
                        } else if constexpr (std::is_same_v<decltype(ptr), float*>) {
                            auto sized_format = predict_sized_format<float>(image.channels);
                            auto format = predict_format_from_sized(sized_format);

                            glTexStorage2D(GL_TEXTURE_2D, levels, sized_format, image.width, image.height);
                            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width, image.height, format,
                                OpenGLType<float>::type, ptr);
                        } else {
                            static_assert(always_false<decltype(ptr)>, "non-exhaustive visitor!");
                        }
                    },
                    image.data);

                if (generate_mipmaps) {
                    glGenerateMipmap(GL_TEXTURE_2D);
                }
            } else {
                k2::Log::core().critical("Invalid image while loading texture");
                throw std::invalid_argument("Invalid image.");
            }
        }
        return *this;
    }
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

    TextureCube& load(const std::array<std::filesystem::path, 6>& texture_paths) {
        if (id == 0) {
            glGenTextures(1, &id);
            glBindTexture(GL_TEXTURE_CUBE_MAP, id);

            for (size_t i = 0; i < texture_paths.size(); i++) {
                auto& texture_path = texture_paths[i];

                Image image { texture_path };

                if (image) {

                    std::visit(
                        [&](auto& image_data) {
                            auto ptr = image_data.get();
                            if constexpr (std::is_same_v<decltype(ptr), std::uint8_t*>) {
                                auto sized_format = Texture2D::predict_sized_format<uint8_t>(image.channels);
                                auto format = Texture2D::predict_format_from_sized(sized_format);
                                glTexImage2D((GLenum)(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 0, format, image.width,
                                    image.height, 0, format, GL_UNSIGNED_BYTE, ptr);
                            } else if constexpr (std::is_same_v<decltype(ptr), float*>) {
                                auto sized_format = Texture2D::predict_sized_format<float>(image.channels);
                                auto format = Texture2D::predict_format_from_sized(sized_format);
                                glTexImage2D((GLenum)(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 0, format, image.width,
                                    image.height, 0, format, GL_FLOAT, ptr);
                            } else {
                                static_assert(always_false<decltype(ptr)>, "non-exhaustive visitor!");
                            }
                        },
                        image.data);

                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                } else {
                    k2::Log::core().critical(std::format("Failed to load image at path: {}", texture_path.string()));
                }
            }
        }
        return *this;
    }
};
}
