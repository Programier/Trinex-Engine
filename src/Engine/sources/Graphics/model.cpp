#include <GL/glew.h>
#include <Graphics/model.hpp>
#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>
#include <stdexcept>


struct GlobalIndexAndImage {
    std::vector<int> _M_index;
    std::vector<Engine::Image*> _M_images;
};

struct EqualSizeListStruct {
    struct ImageSize {
        int w = 0, h = 0;
    } _M_size;

    GlobalIndexAndImage _M_global_index;
};

namespace Engine
{
    bool Model::pair::empty()
    {
        return _M_texture == nullptr && _M_mesh == nullptr;
    }

    void Model::load_textures(const std::vector<std::pair<std::string, const char*>>& names,
                              const bool& invert_textures)
    {

        _M_meshes.clear();
        _M_textures.clear();
        _M_parts.clear();
        _M_parts.reserve(names.size());

        std::unordered_map<std::string, pair> _M_loaded;

        for (const auto& _name : names)
        {
            auto& name = _name.first;
            auto& current_pair = _M_loaded[name];
            if (current_pair.empty())
            {
                std::clog << "Model loader: Loading texture from " << name << std::endl;
                _M_textures.emplace_back();
                bool has_exception = false;
                try
                {
                    _M_textures.back().load(name, _M_mode, invert_textures);
                }
                catch (...)
                {
                    std::cerr << "Model loader: Failed to load " << name << ", model name is " << _name.second
                              << std::endl;
                    _M_textures.pop_back();
                    has_exception = true;
                }

                if (!has_exception)
                {
                    _M_meshes.emplace_back();
                    current_pair._M_mesh = &_M_meshes.back();
                    current_pair._M_texture = &_M_textures.back();
                }
            }

            _M_parts.push_back(current_pair);
        }
    }

    Model& Model::load_model(const std::string& model_file, const DrawMode& mode, const bool& invert)
    {
        _M_mode = mode;
        const std::string directory = model_file.substr(0, model_file.find_last_of('/')) + "/";

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(model_file, aiProcess_Triangulate);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::clog << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return *this;
        }

        // Getting filenames of textures
        {
            std::vector<std::pair<std::string, const char*>> names;
            names.reserve(scene->mNumMaterials);

            for (unsigned int i = 0; i < scene->mNumMaterials; i++)
            {
                auto mat = scene->mMaterials[i];

                aiString name;

                mat->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &name);
                std::string tmp(name.C_Str());

                if (tmp == "")
                {
                    name.Clear();
                    mat->GetTexture(aiTextureType_SPECULAR, 0, &name);
                    tmp = std::string(name.C_Str());
                }

                if (tmp == "")
                {
                    name.Clear();
                    mat->GetTexture(aiTextureType_HEIGHT, 0, &name);
                    tmp = std::string(name.C_Str());
                }

                if (tmp == "")
                {
                    name.Clear();
                    mat->GetTexture(aiTextureType_AMBIENT, 0, &name);
                    tmp = std::string(name.C_Str());
                }


                names.push_back({directory + tmp, mat->GetName().C_Str()});
            }

            // Loading textures
            load_textures(names, invert);
        }

        std::clog << "Model loader: Generating meshes" << std::endl;
        // Generating meshes
        auto meshes_count = scene->mNumMeshes;
        for (decltype(meshes_count) i = 0; i < meshes_count; i++)
        {
            auto& scene_mesh = scene->mMeshes[i];
            auto& model_mesh = _M_parts[scene_mesh->mMaterialIndex];
            if (model_mesh._M_mesh == nullptr)
                continue;
            // [x y z tx ty tz]
            auto face_count = scene_mesh->mNumFaces;
            for (decltype(face_count) j = 0; j < face_count; j++)
            {
                auto& face = scene_mesh->mFaces[j];
                for (unsigned int f = 0; f < face.mNumIndices; f++)
                {
                    auto& vert = scene_mesh->mVertices[face.mIndices[f]];
                    if (scene_mesh->mTextureCoords[0] == nullptr)
                        continue;

                    auto uv = scene_mesh->mTextureCoords[0][face.mIndices[f]];
                    (*model_mesh._M_mesh).data().push_back(vert.x);
                    (*model_mesh._M_mesh).data().push_back(vert.y);
                    (*model_mesh._M_mesh).data().push_back(vert.z);

                    (*model_mesh._M_mesh).data().push_back(uv.x);
                    (*model_mesh._M_mesh).data().push_back(uv.y);
                }
            }
        }

        for (auto& model_mesh : _M_meshes)
            model_mesh.attributes({3, 2}).vertices_count(model_mesh.data().size() / 5).update_buffers();

        return *this;
    }

    Model::~Model()
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    Model::Model() = default;
    Model& Model::draw()
    {
        auto mesh_iterator = _M_meshes.begin();
        auto texture_iterator = _M_textures.begin();

        auto mesh_end = _M_meshes.end();
        auto textures_end = _M_textures.end();

        while (mesh_iterator != mesh_end && texture_iterator != textures_end)
        {
            (*texture_iterator++).bind();
            (*mesh_iterator++).draw(TRIANGLE);
        }

        return *this;
    }

    const DrawMode& Model::mode()
    {
        return _M_mode;
    }

    Model& Model::mode(const DrawMode& mode)
    {
        _M_mode = mode;
        for (auto& texture : _M_textures) texture.draw_mode(mode);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        return *this;
    }

    Model::Model(const std::string& model_file, const DrawMode& mode, const bool& invert)
    {
        load_model(model_file, mode, invert);
    }


}// namespace Engine
