#pragma once
#include "glad/glad.h"

#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <concepts>

namespace k2 {

    struct Shader {
        GLuint handle{};
        GLenum type{};

        Shader(GLenum shader_type, const std::string& source) : type{shader_type}{
            handle = glCreateShader(type);
            const char * ptr[] = {source.c_str()};
            auto size = static_cast<GLint>(source.size());
            glShaderSource(handle, 1, ptr, &size);
            glCompileShader(handle);
        }

        Shader(const GLenum shader_type, const std::filesystem::path& path) : type{shader_type} {
            std::ifstream file(path, std::ios::binary);
            if (file){
                std::string source((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());

                handle = glCreateShader(type);
                const char * ptr[] = {source.c_str()};
                auto size = static_cast<GLint>(source.size());
                glShaderSource(handle, 1, ptr, &size);
                glCompileShader(handle);
            }
        }

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        Shader(Shader&& other) { *this = std::move(other);}

        Shader& operator=(Shader&& other) {
            std::swap(other.handle, handle);
            type = other.type;
            return *this;
        }

        operator bool() const {
            GLint ret{};
            glGetShaderiv(handle, GL_COMPILE_STATUS, &ret);
            return ret != GL_FALSE;
        }

        std::optional<std::string> error_msg() const {
            if (!*this) {
                GLint length{};
                glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &length);

                std::string error_msg(length, '\0');
                GLint new_length{};
                glGetShaderInfoLog(handle, length, &new_length, error_msg.data());
                error_msg.resize(new_length);
                return error_msg;
            }
            return {};
        }

        ~Shader() {
            glDeleteShader(handle);
        }

    };


    struct Program {
        GLuint handle{};

        Program() : handle {glCreateProgram()}{ }

        Program(const Program&) = delete;
        Program& operator=(const Program&) = delete;

        Program(Program&& other) { *this = std::move(other);}

        Program& operator=(Program&& other) {
            std::swap(other.handle, handle);
        }

        template<std::same_as<Shader>... T>
        Program(T&&... shaders) {
            handle = glCreateProgram();
            (attach_shader(std::forward<T>(shaders)), ...);
        }

        void attach_shader(const Shader& shader) {
            glAttachShader(handle, shader.handle);
        }

        void set_uniform(const std::string& str, float f) const {
            glUniform1f(glGetUniformLocation(handle, str.c_str()), f);
        }

        void set_uniform(const std::string& str, glm::vec3 vec) const {
            glUniform3fv(glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
        }

        void set_uniform(const std::string& str, glm::mat4 mat) const {
            glUniformMatrix4fv(glGetUniformLocation(handle, str.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
        }

        void link() {
            glLinkProgram(handle);
        }

        void use() {
            glUseProgram(handle);
        }

        operator bool() const {
            GLint ret{};
            glGetProgramiv(handle, GL_LINK_STATUS, &ret);
            return ret != GL_FALSE;
        }

        std::optional<std::string> error_msg() const {
            if (!*this) {
                GLint length{};
                glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &length);

                std::string error_msg(length, '\0');
                GLint new_length{};
                glGetProgramInfoLog(handle, length, &new_length, error_msg.data());
                error_msg.resize(new_length);
                return error_msg;
            }
            return {};
        }

        ~Program(){
            glDeleteProgram(handle);
        }
    };
}