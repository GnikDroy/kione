#include "rendering/shader.hpp"

#include <fstream>

#include <glm/gtc/type_ptr.hpp>

namespace k2 {

Shader::Shader(std::uint32_t shader_type, const std::string& source)
    : type { shader_type } {
    handle = glCreateShader(type);
    const char* ptr[] = { source.c_str() };
    auto size = static_cast<GLint>(source.size());
    glShaderSource(handle, 1, ptr, &size);
    glCompileShader(handle);
}

Shader::Shader(std::uint32_t shader_type, const std::filesystem::path& path)
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

Shader::operator bool() const {
    std::int32_t ret {};
    glGetShaderiv(handle, GL_COMPILE_STATUS, &ret);
    return ret != GL_FALSE;
}

std::optional<std::string> Shader::error_msg() const {
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

Shader::~Shader() { glDeleteShader(handle); }

void Program::attach_shader(const Shader& shader) {
    if (handle == 0u) {
        handle = glCreateProgram();
    }
    glAttachShader(handle, shader.handle);
}

const Program& Program::set_uniform(const std::string& str, int i) const {
    glProgramUniform1i(handle, glGetUniformLocation(handle, str.c_str()), i);
    return *this;
}

const Program& Program::set_uniform(const std::string& str, glm::ivec2 vec) const {
    glProgramUniform2iv(handle, glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
    return *this;
}

const Program& Program::set_uniform(const std::string& str, glm::ivec3 vec) const {
    glProgramUniform3iv(handle, glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
    return *this;
}

const Program& Program::set_uniform(const std::string& str, glm::ivec4 vec) const {
    glProgramUniform4iv(handle, glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
    return *this;
}

const Program& Program::set_uniform(const std::string& str, unsigned int i) const {
    glProgramUniform1ui(handle, glGetUniformLocation(handle, str.c_str()), i);
    return *this;
}

const Program& Program::set_uniform(const std::string& str, glm::uvec2 vec) const {
    glProgramUniform2uiv(handle, glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
    return *this;
}

const Program& Program::set_uniform(const std::string& str, glm::uvec3 vec) const {
    glProgramUniform3uiv(handle, glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
    return *this;
}

const Program& Program::set_uniform(const std::string& str, glm::uvec4 vec) const {
    glProgramUniform4uiv(handle, glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
    return *this;
}

const Program& Program::set_uniform(const std::string& str, float f) const {
    glProgramUniform1f(handle, glGetUniformLocation(handle, str.c_str()), f);
    return *this;
}

const Program& Program::set_uniform(const std::string& str, glm::vec2 vec) const {
    glProgramUniform2fv(handle, glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
    return *this;
}

const Program& Program::set_uniform(const std::string& str, glm::vec3 vec) const {
    glProgramUniform3fv(handle, glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
    return *this;
}

const Program& Program::set_uniform(const std::string& str, glm::vec4 vec) const {
    glProgramUniform4fv(handle, glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
    return *this;
}

const Program& Program::set_uniform(const std::string& str, double f) const {
    glProgramUniform1d(handle, glGetUniformLocation(handle, str.c_str()), f);
    return *this;
}

const Program& Program::set_uniform(const std::string& str, glm::dvec2 vec) const {
    glProgramUniform2dv(handle, glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
    return *this;
}

const Program& Program::set_uniform(const std::string& str, glm::dvec3 vec) const {
    glProgramUniform3dv(handle, glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
    return *this;
}

const Program& Program::set_uniform(const std::string& str, glm::dvec4 vec) const {
    glProgramUniform4dv(handle, glGetUniformLocation(handle, str.c_str()), 1, glm::value_ptr(vec));
    return *this;
}

const Program& Program::set_uniform(const std::string& str, glm::mat2 mat) const {
    glProgramUniformMatrix2fv(handle, glGetUniformLocation(handle, str.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    return *this;
}

const Program& Program::set_uniform(const std::string& str, glm::mat3 mat) const {
    glProgramUniformMatrix3fv(handle, glGetUniformLocation(handle, str.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    return *this;
}

const Program& Program::set_uniform(const std::string& str, glm::mat4 mat) const {
    glProgramUniformMatrix4fv(handle, glGetUniformLocation(handle, str.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    return *this;
}

const Program& Program::set_uniform(const std::string& str, glm::dmat2 mat) const {
    glProgramUniformMatrix2dv(handle, glGetUniformLocation(handle, str.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    return *this;
}

const Program& Program::set_uniform(const std::string& str, glm::dmat3 mat) const {
    glProgramUniformMatrix3dv(handle, glGetUniformLocation(handle, str.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    return *this;
}

const Program& Program::set_uniform(const std::string& str, glm::dmat4 mat) const {
    glProgramUniformMatrix4dv(handle, glGetUniformLocation(handle, str.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    return *this;
}

void Program::link() const {
    if (handle)
        glLinkProgram(handle);
}

const Program& Program::use() const {
    if (handle != 0u) {
        glUseProgram(handle);
    }
    return *this;
}

Program::operator bool() const {
    GLint ret {};
    glGetProgramiv(handle, GL_LINK_STATUS, &ret);
    return ret != GL_FALSE;
}

std::optional<std::string> Program::error_msg() const {
    if ((handle != 0u) && !*this) {
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

Program::~Program() { glDeleteProgram(handle); }

}
