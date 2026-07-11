#include "rendering/texture.hpp"

#include <cmath>
#include <format>
#include <stdexcept>

#include "core/logger.hpp"
#include "core/utils.hpp"

namespace k2 {

template <FloatOrUInt8 T> GLint Texture2D::predict_sized_format(std::size_t channels) {
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

template GLint Texture2D::predict_sized_format<uint8_t>(std::size_t);
template GLint Texture2D::predict_sized_format<float>(std::size_t);

GLenum Texture2D::predict_format_from_sized(std::size_t sized) {
    switch (sized) {
    case GL_R8:
    case GL_R32F: return GL_RED;

    case GL_RGB8:
    case GL_RGB32F: return GL_RGB;

    case GL_RGBA8:
    case GL_RGBA32F: return GL_RGBA;

    case GL_DEPTH24_STENCIL8: return GL_DEPTH_STENCIL;
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT32: return GL_DEPTH_COMPONENT;
    case GL_STENCIL_INDEX8: return GL_STENCIL_INDEX;
    default: throw std::invalid_argument("Unsupported sized format received.");
    }
}

GLenum Texture2D::predict_type_from_sized(std::size_t sized) {
    switch (sized) {
    case GL_R8:
    case GL_RGB8:
    case GL_RGBA8:
    case GL_STENCIL_INDEX8: return GL_UNSIGNED_BYTE;

    case GL_R32F:
    case GL_RGB32F:
    case GL_RGBA32F: return GL_FLOAT;

    case GL_DEPTH24_STENCIL8: return GL_UNSIGNED_INT_24_8;
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT32: return GL_UNSIGNED_INT;
    default: throw std::invalid_argument("Unsupported sized format received.");
    }
}

template <FloatOrUInt8 T>
Texture2D::Texture2D(std::size_t width, std::size_t height, std::span<const T> data, GLuint sized_format,
    bool generate_mipmaps) {
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, (GLint)sized_format, (GLsizei)width, (GLsizei)height, 0,
        predict_format_from_sized(sized_format), predict_type_from_sized(sized_format), nullptr);
    if (!generate_mipmaps) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    }
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

template Texture2D::Texture2D(std::size_t, std::size_t, std::span<const uint8_t>, GLuint, bool);
template Texture2D::Texture2D(std::size_t, std::size_t, std::span<const float>, GLuint, bool);

template <FloatOrUInt8 T> Texture2D Texture2D::create_white_texture() {
    Texture2D texture;
    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    if constexpr (std::same_as<T, uint8_t>) {
        std::array<T, 4> data = { 255, 255, 255, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    } else if constexpr (std::same_as<T, float>) {
        std::array<T, 4> data = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0, GL_RGBA, GL_FLOAT, data.data());
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    return texture;
}

template Texture2D Texture2D::create_white_texture<uint8_t>();
template Texture2D Texture2D::create_white_texture<float>();

Texture2D& Texture2D::load(const Image& image, bool generate_mipmaps) {
    if (id == 0) {
        if (image) {
            glGenTextures(1, &id);
            glBindTexture(GL_TEXTURE_2D, id);

            if (!generate_mipmaps) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            }

            std::visit(
                [&](auto& image_data) {
                    auto ptr = image_data.get();
                    if constexpr (std::is_same_v<decltype(ptr), std::uint8_t*>) {
                        auto sized_format = predict_sized_format<std::uint8_t>(image.channels);
                        auto format = predict_format_from_sized(sized_format);

                        glTexImage2D(GL_TEXTURE_2D, 0, sized_format, image.width, image.height, 0, format,
                            OpenGLType<uint8_t>::type, ptr);
                    } else if constexpr (std::is_same_v<decltype(ptr), float*>) {
                        auto sized_format = predict_sized_format<float>(image.channels);
                        auto format = predict_format_from_sized(sized_format);

                        glTexImage2D(GL_TEXTURE_2D, 0, sized_format, image.width, image.height, 0, format,
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

TextureCube& TextureCube::load(const std::array<std::filesystem::path, 6>& texture_paths) {
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
                            static_assert(k2::always_false<decltype(ptr)>, "non-exhaustive visitor!");
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

}
