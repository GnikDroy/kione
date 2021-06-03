#pragma once

#include "fmt/format.h"
#include "glad/glad.h"

#include "core/logger.hpp"
#include "core/rendering/image.hpp"

namespace k2 {
    struct Texture2D {
        enum class Type {
            Diffuse,
            Specular,
            Normal,
            Unknown,
        };

        GLuint id{};
        Type type = Type::Unknown;
        std::filesystem::path path;

        Texture2D() = default;

        Texture2D(const Texture2D&) = delete;
        Texture2D& operator=(const Texture2D&) = delete;

        Texture2D(Texture2D&& other) { *this = std::move(other); }

        Texture2D& operator=(Texture2D&& other) {
            std::swap(other.id, id);
            std::swap(other.path, path);
            type = other.type;
            return *this;
        }

        ~Texture2D() {
            glDeleteTextures(1, &id);
        }

        explicit Texture2D(const std::filesystem::path &path) : path(path) {
            load(path);
        }

        operator bool() const {
            return id != 0;
        }

        Texture2D& load(const std::filesystem::path &texture_path) {
            if (id == 0) {
                Image image{texture_path};

                if (image) {
                    glGenTextures(1, &id);
                    glBindTexture(GL_TEXTURE_2D, id);

                    GLint format = [&]() {
                        if (image.channels == 1) return GL_RED;
                        if (image.channels == 3) return GL_RGB;
                        if (image.channels == 4) return GL_RGBA;
                        k2::Logger::core->warn(fmt::format("Incorrect number of channels ({}) for image: {}",
                                                           image.channels,
                                                           texture_path));
                        return GL_RGBA;
                    }();

                    glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0, format, GL_UNSIGNED_BYTE,
                                 image.data);
                    glGenerateMipmap(GL_TEXTURE_2D);

                } else {
                    k2::Logger::core->critical(fmt::format("Failed to load image at path: {}", texture_path.string()));
                }
            }
            return *this;
        }
    };

    struct TextureCube {
        GLuint id{};
        std::array<std::filesystem::path, 6> paths;

        TextureCube() = default;

        TextureCube(const TextureCube&) = delete;
        TextureCube& operator=(const TextureCube&) = delete;

        TextureCube(TextureCube&& other) { *this = std::move(other); }

        TextureCube& operator=(TextureCube&& other) {
            std::swap(other.id, id);
            std::swap(other.paths, paths);
            return *this;
        }

        ~TextureCube() {
            glDeleteTextures(1, &id);
        }

        explicit TextureCube(const std::array<std::filesystem::path, 6> &paths) : paths(paths) {
            load(paths);
        }

        operator bool() const {
            return id != 0;
        }

        TextureCube& load(const std::array<std::filesystem::path, 6>& texture_paths) {
            if (id == 0) {
                glGenTextures(1, &id);
                glBindTexture(GL_TEXTURE_CUBE_MAP, id);

                for (size_t i = 0; i < texture_paths.size(); i++) {
                    auto& texture_path = texture_paths[i];

                    Image image{texture_path};

                    if (image) {
                        GLint format = [&]() {
                            if (image.channels == 1) return GL_RED;
                            if (image.channels == 3) return GL_RGB;
                            if (image.channels == 4) return GL_RGBA;
                            k2::Logger::core->warn(fmt::format("Incorrect number of channels ({}) for image: {}",
                                                               image.channels,
                                                               texture_path));
                            return GL_RGBA;
                        }();

                        glTexImage2D((GLenum)(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 0, format, image.width, image.height, 0, format,
                                     GL_UNSIGNED_BYTE,
                                     image.data);

                        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                    } else {
                        k2::Logger::core->critical(
                                fmt::format("Failed to load image at path: {}", texture_path.string()));
                    }
                }
            }
            return *this;
        }
    };
}

