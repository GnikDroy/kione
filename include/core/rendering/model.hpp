#pragma once
#include <filesystem>
#include <vector>
#include <string>

#include "core/rendering/mesh.hpp"

#include "glm/glm.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "core/fnv.hpp"
#include "core/resource_container.hpp"


namespace k2 {
    class Model {
        std::vector<Mesh> meshes;
        ResourceContainer<Texture> textures;
        std::filesystem::path path;
    public:
        Model() = default;

        explicit Model(const std::filesystem::path& path) : path{path}{
            Assimp::Importer importer;
            const auto scene = importer.ReadFile(path.string().c_str(),
                                                 aiProcess_Triangulate
                                                 | aiProcess_FlipUVs
                                                 | aiProcess_GenNormals);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                return;
            }
            process_node(scene->mRootNode, scene);
        }

        void draw(const Program& program) {
            for (const auto& mesh: meshes) {
                mesh.draw(program, textures);
            }
        }

    private:
        void process_node(aiNode * node, const aiScene *scene) {
            // process meshes
            for(size_t i = 0; i < node->mNumMeshes; i++) {
                meshes.push_back(process_mesh(scene->mMeshes[node->mMeshes[i]], scene));
            }
            // process children
            for(size_t i = 0; i < node->mNumChildren; i++) {
                process_node(node->mChildren[i], scene);
            }
        }

        Mesh process_mesh(aiMesh *ai_mesh, const aiScene *scene) {
            Mesh mesh;

            // process vertices
            for(size_t i = 0; i < ai_mesh->mNumVertices; i++) {
                Vertex vertex{
                        .position{ glm::vec3( ai_mesh->mVertices[i].x,
                                              ai_mesh->mVertices[i].y,
                                              ai_mesh->mVertices[i].z ) },
                        .normal{ glm::vec3( ai_mesh->mNormals[i].x,
                                            ai_mesh->mNormals[i].y,
                                            ai_mesh->mNormals[i].z ) },
                };

                if(ai_mesh->mTextureCoords[0]) {
                    vertex.tex_coord = glm::vec2{ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y};
                }
                mesh.vertices.push_back(vertex);
            }

            // process indices
            for(size_t i = 0; i < ai_mesh->mNumFaces; i++) {
                auto& face = ai_mesh->mFaces[i];
                for(size_t j = 0; j < face.mNumIndices; j++) {
                    mesh.indices.push_back(face.mIndices[j]);
                }
            }

            // process material
            if(ai_mesh->mMaterialIndex >= 0) {
                auto material = scene->mMaterials[ai_mesh->mMaterialIndex];
                load_material_textures(material, aiTextureType_DIFFUSE, std::back_inserter(mesh.textures));
                load_material_textures(material, aiTextureType_SPECULAR, std::back_inserter(mesh.textures));
            }
            mesh.load();
            return mesh;
        }

        template<class OutIt>
        void load_material_textures(aiMaterial *mat, aiTextureType type, OutIt it) {
            for(size_t i = 0; i < mat->GetTextureCount(type); i++) {
                aiString str;
                mat->GetTexture(type, static_cast<unsigned int>(i), &str);
                auto path_to_texture = (path.parent_path() / str.C_Str()).string();

                if (!textures.contains(fnv1a(path_to_texture))){
                    Texture texture{path_to_texture};
                    if (type == aiTextureType_DIFFUSE){
                        texture.type = Texture::Type::Diffuse;
                    } else if (type == aiTextureType_SPECULAR) {
                        texture.type = Texture::Type::Specular;
                    }
                    textures[fnv1a(path_to_texture)] = std::move(texture);
                }
                *(it++) = fnv1a(path_to_texture);
            }
        }

    };
}