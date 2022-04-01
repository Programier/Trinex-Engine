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

    void Model::load_textures(const std::vector<std::pair<std::string, const char*>>& names, const unsigned int& mipmap,
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
                std::clog << "Model loader: Loading texture from " << name << ", model name is " << _name.second
                          << std::endl;
                _M_textures.emplace_back();
                bool has_exception = false;
                try
                {
                    _M_textures.back().load(name, _M_mode, mipmap, invert_textures);
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
#define check_vert(v)                                                                                                  \
    if (_M_limits.min.v > vert.v)                                                                                      \
    {                                                                                                                  \
        _M_limits.min.v = vert.v;                                                                                      \
    }                                                                                                                  \
    if (_M_limits.max.v < vert.v)                                                                                      \
    {                                                                                                                  \
        _M_limits.max.v = vert.v;                                                                                      \
    }
    Model& Model::load_model(const std::string& model_file, const DrawMode& mode, const unsigned int& mipmap,
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

        static const aiTextureType texture_types[] = {
                aiTextureType_AMBIENT,        aiTextureType_AMBIENT_OCCLUSION, aiTextureType_BASE_COLOR,
                aiTextureType_DIFFUSE,        aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_DISPLACEMENT,
                aiTextureType_EMISSION_COLOR, aiTextureType_EMISSIVE,          aiTextureType_HEIGHT,
                aiTextureType_LIGHTMAP,       aiTextureType_METALNESS,         aiTextureType_METALNESS,
                aiTextureType_NORMALS,        aiTextureType_NORMAL_CAMERA,     aiTextureType_OPACITY,
                aiTextureType_REFLECTION,     aiTextureType_SHININESS,         aiTextureType_SPECULAR,
                aiTextureType_UNKNOWN};

        // Getting filenames of textures
        {
            std::vector<std::pair<std::string, const char*>> names;
            names.reserve(scene->mNumMaterials);

            for (unsigned int i = 0; i < scene->mNumMaterials; i++)
            {
                auto mat = scene->mMaterials[i];

                aiString name;
                std::string tmp;
                for (auto& type : texture_types)
                {
                    name.Clear();
                    mat->GetTexture(type, 0, &name);
                    tmp = std::string(name.C_Str());
                    if (tmp != "")
                        break;
                }

                names.push_back({directory + tmp, mat->GetName().C_Str()});
            }

            // Loading textures
            load_textures(names, mipmap, invert);
        }

        bool start = true;
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

                    // Vertex coords
                    auto uv = scene_mesh->mTextureCoords[0][face.mIndices[f]];
                    if (start)
                    {
                        start = false;
                        _M_limits.max = _M_limits.min = {vert.x, vert.y, vert.z};
                    }
                    else
                    {
                        check_vert(x);
                        check_vert(y);
                        check_vert(z);
                    }
                    (*model_mesh._M_mesh).data().push_back(vert.x);
                    (*model_mesh._M_mesh).data().push_back(vert.y);
                    (*model_mesh._M_mesh).data().push_back(vert.z);

                    // Texture coords
                    (*model_mesh._M_mesh).data().push_back(uv.x);
                    (*model_mesh._M_mesh).data().push_back(uv.y);

                    // Normals
                    if (scene_mesh->mNormals != nullptr)
                    {
                        auto& normal = scene_mesh->mNormals[face.mIndices[f]];
                        (*model_mesh._M_mesh).data().push_back(normal.x);
                        (*model_mesh._M_mesh).data().push_back(normal.y);
                        (*model_mesh._M_mesh).data().push_back(normal.z);
                    }
                    else
                    {
                        (*model_mesh._M_mesh).data().push_back(0);
                        (*model_mesh._M_mesh).data().push_back(0);
                        (*model_mesh._M_mesh).data().push_back(0);
                    }
                }
            }
        }

        for (auto& model_mesh : _M_meshes)
            model_mesh.attributes({3, 2, 3}).vertices_count(model_mesh.data().size() / 8).update_buffers();

        std::clog << "Model loader: Loading the \"" << model_file << "\" model completed successfully" << std::endl;
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

    Model::Model(const std::string& model_file, const DrawMode& mode, const unsigned int& mipmap, const bool& invert)
    {
        load_model(model_file, mode, mipmap, invert);
    }

    const std::list<Texture>& Model::textures() const
    {
        return _M_textures;
    }

    const std::list<Mesh>& Model::meshes() const
    {
        return _M_meshes;
    }

    const std::vector<Model::pair>& Model::parts() const
    {
        return _M_parts;
    }

    const Model::Limits& Model::limits() const
    {
        return _M_limits;
    }


}// namespace Engine
