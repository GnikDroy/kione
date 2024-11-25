#pragma once

#include "core/fnv.hpp"
#include "core/logger.hpp"
#include "core/resources.hpp"
#include "mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace k2 {
class Model {
    std::vector<Mesh> meshes;
    std::filesystem::path path;

public:
    Model() = default;

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    explicit Model(const std::filesystem::path& path)
        : path { path } {
        Assimp::Importer importer;
        const auto scene = importer.ReadFile(path.string().c_str(),
            aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            k2::Log::core().warn(importer.GetErrorString());
            return;
        }

        k2::Log::core().trace(std::format("Loaded model: {}", path.string()));
        process_node(scene->mRootNode, scene);
        auto num_meshes = meshes.size();
        auto num_vertices = std::accumulate(
            meshes.begin(), meshes.end(), size_t(0), [](auto n, const Mesh& mesh) { return n + mesh.vertices.size(); });
        k2::Log::core().trace(std::format("Model contains: {} meshes and {} vertices", num_meshes, num_vertices));
    }

    void draw(const Program& program) {
        for (const auto& mesh : meshes) {
            mesh.draw(program);
        }
    }

private:
    void process_node(aiNode* node, const aiScene* scene) {
        // process meshes
        for (size_t i = 0; i < node->mNumMeshes; i++) {
            meshes.push_back(process_mesh(scene->mMeshes[node->mMeshes[i]], scene));
        }
        // process children
        for (size_t i = 0; i < node->mNumChildren; i++) {
            process_node(node->mChildren[i], scene);
        }
    }

    Mesh process_mesh(aiMesh* ai_mesh, const aiScene* scene) {
        // process vertices
        std::vector<Vertex> vertices;
        for (size_t i = 0; i < ai_mesh->mNumVertices; i++) {
            Vertex vertex { .position {
                                glm::vec3(ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z) },
                .normal { glm::vec3(ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y, ai_mesh->mNormals[i].z) },
                .tangent { glm::vec3(ai_mesh->mTangents[i].x, ai_mesh->mTangents[i].y, ai_mesh->mTangents[i].z) } };

            if (ai_mesh->mTextureCoords[0] != nullptr) {
                vertex.tex_coord = glm::vec2 { ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y };
            }
            vertices.push_back(vertex);
        }

        // process indices
        std::vector<unsigned int> indices;
        for (size_t i = 0; i < ai_mesh->mNumFaces; i++) {
            auto& face = ai_mesh->mFaces[i];
            for (size_t j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        // process material
        std::vector<std::uint64_t> textures_vec;
        if (ai_mesh->mMaterialIndex >= 0) {
            auto* material = scene->mMaterials[ai_mesh->mMaterialIndex];
            load_material_textures(material, aiTextureType_DIFFUSE, std::back_inserter(textures_vec));
            load_material_textures(material, aiTextureType_SPECULAR, std::back_inserter(textures_vec));
            load_material_textures(material, aiTextureType_NORMALS, std::back_inserter(textures_vec));
        }
        return { vertices, indices, textures_vec };
    }

    template <class OutIt> void load_material_textures(aiMaterial* mat, aiTextureType type, OutIt it) {
        for (size_t i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, static_cast<unsigned int>(i), &str);
            auto path_to_texture = (path.parent_path() / str.C_Str()).string();

            if (!Resources::get<Texture2D>().contains(fnv1a(path_to_texture))) {
                Texture2D texture { Image(path_to_texture) };
                if (type == aiTextureType_DIFFUSE) {
                    texture.type = Texture2D::Type::Diffuse;
                } else if (type == aiTextureType_SPECULAR) {
                    texture.type = Texture2D::Type::Specular;
                } else if (type == aiTextureType_NORMALS) {
                    texture.type = Texture2D::Type::Normal;
                }
                Resources::get<Texture2D>()[fnv1a(path_to_texture)] = std::move(texture);
            }
            *(it++) = fnv1a(path_to_texture);
        }
    }
};
}
