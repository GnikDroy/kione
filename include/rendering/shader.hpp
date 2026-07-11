#pragma once

#include <concepts>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <utility>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "core/utils.hpp"

namespace k2 {

struct Shader {
    std::uint32_t handle {};
    std::uint32_t type {};

    Shader(std::uint32_t shader_type, const std::string& source);
    Shader(std::uint32_t shader_type, const std::filesystem::path& path);

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other) noexcept { *this = std::move(other); }

    Shader& operator=(Shader&& other) noexcept {
        std::swap(other.handle, handle);
        type = other.type;
        return *this;
    }

    operator bool() const;

    [[nodiscard]] std::optional<std::string> error_msg() const;

    ~Shader();
};

struct Program {
    GLuint handle {};

    Program() = default;

    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;

    Program(Program&& other) noexcept { *this = std::move(other); }

    Program& operator=(Program&& other) noexcept {
        std::swap(other.handle, handle);
        return *this;
    }

    template <std::same_as<Shader>... T> Program(T&&... shaders) {
        handle = glCreateProgram();
        (attach_shader(std::forward<T>(shaders)), ...);
    }

    void attach_shader(const Shader& shader);

    const Program& set_uniform(const std::string& str, int i) const;
    const Program& set_uniform(const std::string& str, glm::ivec2 vec) const;
    const Program& set_uniform(const std::string& str, glm::ivec3 vec) const;
    const Program& set_uniform(const std::string& str, glm::ivec4 vec) const;
    const Program& set_uniform(const std::string& str, unsigned int i) const;
    const Program& set_uniform(const std::string& str, glm::uvec2 vec) const;
    const Program& set_uniform(const std::string& str, glm::uvec3 vec) const;
    const Program& set_uniform(const std::string& str, glm::uvec4 vec) const;
    const Program& set_uniform(const std::string& str, float f) const;
    const Program& set_uniform(const std::string& str, glm::vec2 vec) const;
    const Program& set_uniform(const std::string& str, glm::vec3 vec) const;
    const Program& set_uniform(const std::string& str, glm::vec4 vec) const;
    const Program& set_uniform(const std::string& str, double f) const;
    const Program& set_uniform(const std::string& str, glm::dvec2 vec) const;
    const Program& set_uniform(const std::string& str, glm::dvec3 vec) const;
    const Program& set_uniform(const std::string& str, glm::dvec4 vec) const;
    const Program& set_uniform(const std::string& str, glm::mat2 mat) const;
    const Program& set_uniform(const std::string& str, glm::mat3 mat) const;
    const Program& set_uniform(const std::string& str, glm::mat4 mat) const;
    const Program& set_uniform(const std::string& str, glm::dmat2 mat) const;
    const Program& set_uniform(const std::string& str, glm::dmat3 mat) const;
    const Program& set_uniform(const std::string& str, glm::dmat4 mat) const;

    template <arithmetic T> const Program& set_uniform(const std::string& str, const std::span<T>& vec) const {
        if constexpr (std::is_same_v<T, std::uint32_t>) {
            glProgramUniform1uiv(handle, glGetUniformLocation(handle, str.c_str()), (GLsizei)vec.size(), vec.data());
        } else if constexpr (std::is_same_v<T, std::int32_t>) {
            glProgramUniform1iv(handle, glGetUniformLocation(handle, str.c_str()), (GLsizei)vec.size(), vec.data());
        } else if constexpr (std::is_same_v<T, float>) {
            glProgramUniform1fv(handle, glGetUniformLocation(handle, str.c_str()), (GLsizei)vec.size(), vec.data());
        } else if constexpr (std::is_same_v<T, double>) {
            glProgramUniform1dv(handle, glGetUniformLocation(handle, str.c_str()), (GLsizei)vec.size(), vec.data());
        }
        return *this;
    }

    void link() const;

    const Program& use() const;

    operator bool() const;

    [[nodiscard]] std::optional<std::string> error_msg() const;

    ~Program();
};

enum class ShaderDataType {
    Int,
    Int2,
    Int3,
    Int4,
    Uint,
    Uint2,
    Uint3,
    Uint4,
    Float,
    Float2,
    Float3,
    Float4,
    Mat2x2,
    Mat3x3,
    Mat4x4,
    Double,
    Double2,
    Double3,
    Double4,
    Bool,
};
}
