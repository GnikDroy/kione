#pragma once

#include "fmt/format.h"
#include "glad/glad.h"

#include "core/logger.hpp"
#include "core/rendering/image.hpp"

namespace k2 {
    struct Texture {
        enum class Type {
            Diffuse,
            Specular,
            Unknown,
        };

        GLuint id{};
        Type type = Type::Unknown;
        std::filesystem::path path;

        Texture() = default;

        explicit Texture(const std::filesystem::path &path) : path(path) {
            load(path);
        }

        operator bool() const {
            return id != 0;
        }

        Texture& load(const std::filesystem::path &texture_path) {
            if (id == 0) {
                Image image{texture_path};

                if (image) {
                    glGenTextures(1, &id);
                    GLenum format = [&]() {
                        if (image.channels == 1) return GL_RED;
                        if (image.channels == 3) return GL_RGB;
                        if (image.channels == 4) return GL_RGBA;
                        k2::Logger::core->warn(fmt::format("Incorrect number of channels ({}) for image: {}",
                                                           image.channels,
                                                           texture_path));
                        return GL_RGBA;
                    }();

                    glBindTexture(GL_TEXTURE_2D, id);
                    glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0, format, GL_UNSIGNED_BYTE,
                                 image.data);
                    glGenerateMipmap(GL_TEXTURE_2D);

                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                } else {
                    k2::Logger::core->critical(fmt::format("Failed to load image at path: {}", texture_path));
                }
            }
            return *this;
        }
    };

}

