#pragma once

#include <format>
#include <glad/glad.h>
#include <span>

#include "core/logger.hpp"
#include "image.hpp"

namespace k2 {
struct Texture2D {
private:
    static auto predict_sized_format(std::size_t channels) {
        switch (channels) {
        case 1: return GL_R8;
        case 3: return GL_RGB8;
        case 4: return GL_RGBA8;
        default: {
            k2::Log::core().warn(std::format("Incorrect number of channels ({}) for image, assuming RGBA8.", channels));
            return GL_RGBA8;
        }
        }
    }
    static auto predict_format_from_sized(std::size_t sized) {
        switch (sized) {
        case GL_R8: return GL_RED;
        case GL_RGB8: return GL_RGB;
        case GL_RGBA8: return GL_RGBA;
        case GL_DEPTH24_STENCIL8: return GL_DEPTH_STENCIL;
        case GL_DEPTH_COMPONENT: return GL_DEPTH_COMPONENT;
        case GL_STENCIL_INDEX8: return GL_STENCIL_INDEX;
        default: throw std::invalid_argument("Unsupported sized format received.");
        }
    }

public:
    enum class Type {
        Diffuse,
        Specular,
        Normal,
        Unknown,
    };

    GLuint id {};
    Type type = Type::Unknown;

    Texture2D() = default;

    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    Texture2D(Texture2D&& other) noexcept { *this = std::move(other); }

    Texture2D& operator=(Texture2D&& other) noexcept {
        std::swap(other.id, id);
        type = other.type;
        return *this;
    }

    ~Texture2D() { glDeleteTextures(1, &id); }

    explicit Texture2D(const Image& image, bool generate_mipmaps = true) { load(image, generate_mipmaps); }

    Texture2D(std::size_t width, std::size_t height, std::span<const std::uint8_t> data = {},
        GLuint sized_format = GL_RGBA8, bool generate_mipmaps = true) {
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
                predict_format_from_sized(sized_format), GL_UNSIGNED_BYTE, data.data());
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

    static Texture2D create_white_texture() {
        Texture2D texture;
        glGenTextures(1, &texture.id);
        glBindTexture(GL_TEXTURE_2D, texture.id);
        std::array<std::uint8_t, 4> data = { 0xFF, 0xFF, 0xFF, 0xFF };
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
        return texture;
    }

    Texture2D& load(const Image& image, bool generate_mipmaps = true) {
        if (id == 0) {
            if (image) {
                glGenTextures(1, &id);
                glBindTexture(GL_TEXTURE_2D, id);
                auto sized_format = predict_sized_format(image.channels);
                auto levels = (GLsizei)(std::floor(std::log2(std::max(image.width, image.height))) + 1);
                if (!generate_mipmaps) {
                    levels = 1;
                }
                glTexStorage2D(GL_TEXTURE_2D, levels, sized_format, image.width, image.height);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width, image.height,
                    predict_format_from_sized(sized_format), GL_UNSIGNED_BYTE, image.data);
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
                    GLint format = [&]() {
                        if (image.channels == 1)
                            return GL_RED;
                        if (image.channels == 3)
                            return GL_RGB;
                        if (image.channels == 4)
                            return GL_RGBA;
                        k2::Log::core().warn(std::format(
                            "Incorrect number of channels ({}) for image: {}", image.channels, texture_path.string()));
                        return GL_RGBA;
                    }();

                    glTexImage2D((GLenum)(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 0, format, image.width, image.height, 0,
                        format, GL_UNSIGNED_BYTE, image.data);

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
