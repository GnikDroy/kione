#pragma once

#include <string>
#include <span>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <optional>

#include "bgfx/bgfx.h"
#include "core/logger.hpp"

namespace k2 {

    class Uniform {
        bgfx::UniformHandle handle;
    public:
        Uniform(const char *name, bgfx::UniformType::Enum type, uint16_t num = 1)
                : handle{bgfx::createUniform(name, type, num)} {}

        ~Uniform() {
            bgfx::destroy(handle);
        }

        bgfx::UniformInfo get_info() {
            bgfx::UniformInfo info;
            bgfx::getUniformInfo(handle, info);
            return info;
        }
    };

    class Shader {
        bgfx::ShaderHandle handle;
    public:
        friend class Program;

        Shader(const bgfx::Memory *mem) :
                handle{bgfx::createShader(mem)} {}

        ~Shader() {
            bgfx::destroy(handle);
        }

        static std::optional<Shader> load(const std::filesystem::path& path){
            std::ifstream shader_file(path, std::ios::binary);
            if (shader_file.good()) {
                auto memory = bgfx::alloc(static_cast<uint32_t>(std::filesystem::file_size(path)));
                std::copy(std::istreambuf_iterator<char>(shader_file), std::istreambuf_iterator<char>(), memory->data);
                return {memory};
            }
            Logger::app->info("Shader file not present.");
            return nullptr;
        }

        std::span<bgfx::UniformHandle> get_uniforms() {
            bgfx::UniformHandle *uniform_handles{};
            auto num = bgfx::getShaderUniforms(handle, uniform_handles);
            return {uniform_handles, num};
        }
    };

    class Program {
        bgfx::ProgramHandle handle;
    public:
        Program(const Shader &compute_shader)
                : handle{bgfx::createProgram(compute_shader.handle, false)} {}

        Program(const Shader &vertex_shader, const Shader &fragment_shader)
                : handle{bgfx::createProgram(vertex_shader.handle, fragment_shader.handle, false)} {}

        ~Program() {
            bgfx::destroy(handle);
        }
    };

    class VertexLayout {
        bgfx::VertexLayoutHandle handle;
    public:
        VertexLayout(const bgfx::VertexLayout &layout)
                : handle{bgfx::createVertexLayout(layout)} {}

        ~VertexLayout() {
            bgfx::destroy(handle);
        }
    };

    class VertexBuffer {
        bgfx::VertexBufferHandle handle;
    public:
        VertexBuffer(const bgfx::Memory *mem, const bgfx::VertexLayout &layout, uint16_t flags = BGFX_BUFFER_NONE)
                : handle{bgfx::createVertexBuffer(mem, layout, flags)} {}


        ~VertexBuffer() {
            bgfx::destroy(handle);
        }
    };

    class IndexBuffer {
        bgfx::IndexBufferHandle handle;
    public:
        IndexBuffer(const bgfx::Memory *mem, uint16_t flags = BGFX_BUFFER_NONE)
                : handle{bgfx::createIndexBuffer(mem, flags)} {}

        ~IndexBuffer() {
            bgfx::destroy(handle);
        }
    };
}