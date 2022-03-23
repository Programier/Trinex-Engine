#include "model.hpp"
#include "basic_texturearray.hpp"
#include <GL/glew.h>
#include <algorithm>
#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>
#include <list>
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
    void Model::load_textures(const std::vector<std::pair<std::string, const char*>>& names,
                              const bool& invert_textures)
    {
        _M_models.clear();
        _M_models.resize(names.size());

        _M_images.clear();
        _M_mesh.clear();
        _M_texture_array.clear();
        std::unordered_map<std::string, Image*> _M_loaded;


        std::list<EqualSizeListStruct> _M_equal_size;

        EqualSizeListStruct::ImageSize size;
        auto predicate = [&size](const EqualSizeListStruct& value) {
            return value._M_size.h == size.h && value._M_size.w == size.w;
        };

        // Loading textures
        int current_index = -1;
        for (auto& _M_pair : names)
        {
            auto& name = _M_pair.first;
            current_index++;
            Image* current_image = _M_loaded[name];
            if (current_image != nullptr)
            {
                std::clog << "Model loader: Skipping loading file " << name
                          << ", file already loaded" << std::endl;
            }
            else
            {
                std::clog << "Model loader: Loading file " << name << std::endl;
                _M_images.emplace_back();
                _M_images.back().load(name, invert_textures);
                if (_M_images.back().empty())
                {
                    std::clog << "Model loader: Failed to load " << name << " , skipping model "
                              << _M_pair.second << std::endl;
                    _M_images.pop_back();
                    continue;
                }
                current_image = &_M_images.back();
                _M_loaded[name] = current_image;
            }
            size = {current_image->width(), current_image->height()};
            auto iterator = std::find_if(_M_equal_size.begin(), _M_equal_size.end(), predicate);
            if (iterator == _M_equal_size.end())
            {
                _M_equal_size.push_back({size, {{current_index}, {current_image}}});
            }
            else
            {
                (*iterator)._M_global_index._M_index.push_back(current_index);
                (*iterator)._M_global_index._M_images.push_back(current_image);
            }
        }

        std::clog << "Model loader: Generating texture arrays" << std::endl;
        for (auto& future_texture_array : _M_equal_size)
        {
            _M_mesh.emplace_back();
            _M_texture_array.push_back(Engine::basic_texturearray::gen_texture_array(
                    future_texture_array._M_global_index._M_images,
                    {future_texture_array._M_size.w, future_texture_array._M_size.h}));
            glBindTexture(GL_TEXTURE_2D_ARRAY, _M_texture_array.back());
            auto m = _M_mode == LINEAR ? GL_LINEAR : GL_NEAREST;
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, m);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, m);
            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

            int current_index = 0;
            std::unordered_map<Image*, int> _M_already_indexed;
            auto image_iterator = future_texture_array._M_global_index._M_images.begin();
            for (auto& global_index : future_texture_array._M_global_index._M_index)
            {
                int& tmp = _M_already_indexed[*image_iterator++];
                if (tmp == 0)
                {
                    _M_models[global_index] = {&_M_mesh.back(), current_index};
                    tmp = current_index++;
                }
                else
                {
                    _M_models[global_index] = {&_M_mesh.back(), tmp};
                }
            }
        }
    }

    Model& Model::load_model(const std::string& model_file, const DrawMode& mode,
                             const bool& invert)
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

        // Generating meshes
        auto meshes_count = scene->mNumMeshes;
        for (decltype(meshes_count) i = 0; i < meshes_count; i++)
        {
            auto& scene_mesh = scene->mMeshes[i];
            auto& model_mesh = _M_models[scene_mesh->mMaterialIndex];
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
                    auto& uv = scene_mesh->mTextureCoords[0][face.mIndices[f]];
                    (*model_mesh._M_mesh).data().push_back(vert.x);
                    (*model_mesh._M_mesh).data().push_back(vert.y);
                    (*model_mesh._M_mesh).data().push_back(vert.z);

                    (*model_mesh._M_mesh).data().push_back(uv.x);
                    (*model_mesh._M_mesh).data().push_back(uv.y);
                    (*model_mesh._M_mesh)
                            .data()
                            .push_back(static_cast<float>(model_mesh._M_local_index));
                }
            }
        }

        for (auto& model_mesh : _M_mesh)
        {
            model_mesh.attributes({3, 3})
                    .vertices_count(model_mesh.data().size() / 6)
                    .update_buffers();
        }

        return *this;
    }

    Model::~Model()
    {
        for (auto& ell : _M_texture_array) glDeleteTextures(1, &ell);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }

    Model::Model() = default;
    Model& Model::draw()
    {
        auto mesh_iterator = _M_mesh.begin();
        auto texture_iterator = _M_texture_array.begin();

        auto mesh_end = _M_mesh.end();
        auto texture_end = _M_texture_array.end();
        while (mesh_iterator != mesh_end && texture_iterator != texture_end)
        {
            glBindTexture(GL_TEXTURE_2D_ARRAY, (*texture_iterator++));
            (*mesh_iterator++).draw(Engine::TRIANGLE);
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
        auto m = _M_mode == LINEAR ? GL_LINEAR : GL_NEAREST;
        for (auto& ID : _M_texture_array)
        {
            glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, m);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, m);
        }
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        return *this;
    }

    Model::Model(const std::string& model_file, const DrawMode& mode, const bool& invert)
    {
        load_model(model_file, mode, invert);
    }


}// namespace Engine
