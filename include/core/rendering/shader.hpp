#pragma once

#include <concepts>
#include <filesystem>
#include <fstream>
#include <optional>
#include <span>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/utils.hpp"

namespace k2 {

struct Shader {
    std::uint32_t handle {};
    std::uint32_t type {};

    Shader(std::uint32_t shader_type, const std::string& source)
        : type { shader_type } {
        handle = glCreateShader(type);
        const char* ptr[] = { source.c_str() };
        auto size = static_cast<GLint>(source.size());
        glShaderSource(handle, 1, ptr, &size);
        glCompileShader(handle);
    }

    Shader(const std::uint32_t shader_type, const std::filesystem::path& path)
        : type { shader_type } {
        std::ifstream file(path, std::ios::binary);
        if (file) {
            std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

            handle = glCreateShader(type);
            const char* ptr[] = { source.c_str() };
            auto size = static_cast<GLint>(source.size());
            glShaderSource(handle, 1, ptr, &size);
            glCompileShader(handle);
        }
    }

    Shader(const Shader&) = delete;

    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other) { *this = std::move(other); }

    Shader& operator=(Shader&& other) {
        std::swap(other.handle, handle);
        type = other.type;
        return *this;
    }

    operator bool() const {
        std::int32_t ret {};
        glGetShaderiv(handle, GL_COMPILE_STATUS, &ret);
        return ret != GL_FALSE;
    }

    std::optional<std::string> error_msg() const {
        if (!*this) {
            std::int32_t length {};
            glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &length);

            std::string error_msg(length, '\0');
            std::int32_t new_length {};
            glGetShaderInfoLog(handle, length, &new_length, error_msg.data());
            error_msg.resize(new_length);
            return error_msg;
        }
        return {};
    }

    ~Shader() { glDeleteShader(handle); }
};

struct Program {
    GLuint handle {};

    Program() = default;

    Program(const Program&) = delete;

    Program& operator=(const Program&) = delete;

    Program(Program&& other) { *this = std::move(other); }

    Program& operator=(Program&& other) {
        std::swap(other.handle, handle);
        return *this;
    }

    template <std::same_as<Shader>... T> Program(T&&... shaders) {
        handle = glCreateProgram();
        (attach_shader(std::forward<T>(shaders)), ...);
    }

    void attach_shader(const Shader& shader) {
        if (!handle)
            handle = glCreateProgram();
        glAttachShader(handle, shader.handle);
    }

    const Program& set_uniform(const std::string& str, int i) const {
        glUniform1i(glGetUniformLocation(handle, str.c_str()), i);
        return *this;
    }

    const Program& set_uniform(const std::string& str, glm::ivec2 vec) const {
        glUniform2iv(glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
        return *this;
    }

    const Program& set_uniform(const std::string& str, glm::ivec3 vec) const {
        glUniform3iv(glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
        return *this;
    }

    const Program& set_uniform(const std::string& str, glm::ivec4 vec) const {
        glUniform4iv(glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
        return *this;
    }

    const Program& set_uniform(const std::string& str, unsigned int i) const {
        glUniform1ui(glGetUniformLocation(handle, str.c_str()), i);
        return *this;
    }

    const Program& set_uniform(const std::string& str, glm::uvec2 vec) const {
        glUniform2uiv(glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
        return *this;
    }

    const Program& set_uniform(const std::string& str, glm::uvec3 vec) const {
        glUniform3uiv(glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
        return *this;
    }

    const Program& set_uniform(const std::string& str, glm::uvec4 vec) const {
        glUniform4uiv(glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
        return *this;
    }

    template <arithmetic T> const Program& set_uniform(const std::string& str, const std::span<T>& vec) const {
        if constexpr (std::is_same_v<T, std::uint32_t>) {
            glUniform1uiv(glGetUniformLocation(handle, str.c_str()), (GLsizei)vec.size(), vec.data());
        } else if (std::is_same_v<T, std::int32_t>) {
            glUniform1iv(glGetUniformLocation(handle, str.c_str()), (GLsizei)vec.size(), vec.data());
        } else if (std::is_same_v<T, float>) {
            glUniform1fv(glGetUniformLocation(handle, str.c_str()), (GLsizei)vec.size(), vec.data());
        } else if (std::is_same_v<T, double>) {
            glUniform1dv(glGetUniformLocation(handle, str.c_str()), (GLsizei)vec.size(), vec.data());
        }
        return *this;
    }

    const Program& set_uniform(const std::string& str, float f) const {
        glUniform1f(glGetUniformLocation(handle, str.c_str()), f);
        return *this;
    }

    const Program& set_uniform(const std::string& str, glm::vec2 vec) const {
        glUniform2fv(glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
        return *this;
    }

    const Program& set_uniform(const std::string& str, glm::vec3 vec) const {
        glUniform3fv(glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
        return *this;
    }

    const Program& set_uniform(const std::string& str, glm::vec4 vec) const {
        glUniform4fv(glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
        return *this;
    }

    const Program& set_uniform(const std::string& str, double f) const {
        glUniform1d(glGetUniformLocation(handle, str.c_str()), f);
        return *this;
    }

    const Program& set_uniform(const std::string& str, glm::dvec2 vec) const {
        glUniform2dv(glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
        return *this;
    }

    const Program& set_uniform(const std::string& str, glm::dvec3 vec) const {
        glUniform3dv(glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
        return *this;
    }

    const Program& set_uniform(const std::string& str, glm::dvec4 vec) const {
        glUniform4dv(glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
        return *this;
    }

    const Program& set_uniform(const std::string& str, glm::mat2 mat) const {
        glUniformMatrix2fv(glGetUniformLocation(handle, str.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
        return *this;
    }

    const Program& set_uniform(const std::string& str, glm::mat3 mat) const {
        glUniformMatrix3fv(glGetUniformLocation(handle, str.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
        return *this;
    }

    const Program& set_uniform(const std::string& str, glm::mat4 mat) const {
        glUniformMatrix4fv(glGetUniformLocation(handle, str.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
        return *this;
    }

    const Program& set_uniform(const std::string& str, glm::dmat2 mat) const {
        glUniformMatrix2dv(glGetUniformLocation(handle, str.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
        return *this;
    }

    const Program& set_uniform(const std::string& str, glm::dmat3 mat) const {
        glUniformMatrix3dv(glGetUniformLocation(handle, str.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
        return *this;
    }

    const Program& set_uniform(const std::string& str, glm::dmat4 mat) const {
        glUniformMatrix4dv(glGetUniformLocation(handle, str.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
        return *this;
    }

    void link() const {
        if (handle)
            glLinkProgram(handle);
    }

    const Program& use() const {
        if (handle)
            glUseProgram(handle);
        return *this;
    }

    operator bool() const {
        GLint ret {};
        glGetProgramiv(handle, GL_LINK_STATUS, &ret);
        return ret != GL_FALSE;
    }

    std::optional<std::string> error_msg() const {
        if (handle && !*this) {
            GLint length {};
            glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &length);

            std::string error_msg(length, '\0');
            GLint new_length {};
            glGetProgramInfoLog(handle, length, &new_length, error_msg.data());
            error_msg.resize(new_length);
            return error_msg;
        }
        return {};
    }

    ~Program() { glDeleteProgram(handle); }
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