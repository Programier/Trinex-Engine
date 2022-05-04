#include <GL/glew.h>
#include <Graphics/terrainmodel.hpp>
#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <fstream>
#include <iostream>
#include <stdexcept>

static const aiTextureType texture_types[] = {
        aiTextureType_AMBIENT,        aiTextureType_AMBIENT_OCCLUSION, aiTextureType_BASE_COLOR,
        aiTextureType_DIFFUSE,        aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_DISPLACEMENT,
        aiTextureType_EMISSION_COLOR, aiTextureType_EMISSIVE,          aiTextureType_HEIGHT,
        aiTextureType_LIGHTMAP,       aiTextureType_METALNESS,         aiTextureType_METALNESS,
        aiTextureType_NORMALS,        aiTextureType_NORMAL_CAMERA,     aiTextureType_OPACITY,
        aiTextureType_REFLECTION,     aiTextureType_SHININESS,         aiTextureType_SPECULAR,
        aiTextureType_UNKNOWN};

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
    static glm::mat4 mat4(const aiMatrix4x4& matrix)
    {
        glm::mat4 result;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++) result[i][j] = matrix[i][j];
        return result;
    }

    static glm::vec3 vec3(const aiVector3D& vector)
    {
        return glm::vec3(vector.x, vector.y, vector.z);
    }


#define check_vert(v)                                                                                                                 \
    if (_M_limits.min.v > vert.v)                                                                                                     \
    {                                                                                                                                 \
        _M_limits.min.v = vert.v;                                                                                                     \
    }                                                                                                                                 \
    if (_M_limits.max.v < vert.v)                                                                                                     \
    {                                                                                                                                 \
        _M_limits.max.v = vert.v;                                                                                                     \
    }

    static void load_vertices(const aiScene* scene, aiNode* node, std::vector<TerrainModel::Material>& materials,
                              TerrainModel::Limits& _M_limits, bool& start, glm::mat4 matrix = identity_matrix)
    {
        matrix *= mat4(node->mTransformation.Transpose());
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            auto& ai_mesh = scene->mMeshes[node->mMeshes[i]];
            auto& ai_UV = ai_mesh->mTextureCoords[0];
            auto& mesh = materials[ai_mesh->mMaterialIndex].mesh.data();

            for (unsigned int f = 0; f < ai_mesh->mNumFaces; f++)
            {
                auto& face = ai_mesh->mFaces[f];

                for (unsigned int index = 0; index < face.mNumIndices; index++)
                {
                    auto v_index = face.mIndices[index];
                    auto vert = vec3(ai_mesh->mVertices[v_index]);
                    vert = matrix * glm::vec4(vert, 1.f);
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
                    glm::vec2 uv = ai_UV ? glm::vec2(ai_UV[v_index].x, ai_UV[v_index].y) : glm::vec2(0.f, 0.f);

                    // Pushing values
                    mesh.push_back(vert.x);
                    mesh.push_back(vert.y);
                    mesh.push_back(vert.z);
                    mesh.push_back(uv.x);
                    mesh.push_back(uv.y);

                    // Normals
                    if (ai_mesh->mNormals != nullptr)
                    {
                        auto& normal = ai_mesh->mNormals[face.mIndices[index]];
                        mesh.push_back(normal.x);
                        mesh.push_back(normal.y);
                        mesh.push_back(normal.z);
                    }
                    else
                    {
                        mesh.push_back(0);
                        mesh.push_back(0);
                        mesh.push_back(0);
                    }
                }
            }
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++)
            load_vertices(scene, node->mChildren[i], materials, _M_limits, start, matrix);
    }


    TerrainModel& TerrainModel::load_model(const std::string& model_file, const DrawMode& mode, const unsigned int& mipmap,
                                           const bool& invert)
    {
        std::string path = model_file.substr(0, model_file.find_last_of('/') + 1);
        _M_materials.clear();

        Assimp::Importer importer;
        auto scene = importer.ReadFile(model_file, aiProcess_Triangulate);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::clog << "Terrain loader: Failed to load " << model_file << std::endl;
            return *this;
        }

        std::clog << "Terraing model: Material num: " << scene->mNumMaterials << std::endl;
        _M_materials.resize(scene->mNumMaterials);

        // Loading materials
        for (unsigned int i = 0; i < scene->mNumMaterials; i++)
        {
            auto& ai_material = scene->mMaterials[i];
            auto& material = _M_materials[i];
            material.index = i;
            material.name = ai_material->GetName().C_Str();
            std::clog << "Terrain loader: loading " << material.name << ", index = " << i << std::endl;

            aiString filename;
            for (auto& type : texture_types)
            {
                ai_material->GetTexture(type, 0, &filename);
                if (std::string(filename.C_Str()) != "")
                {
                    std::string std_filename(filename.C_Str());
                    try
                    {
                        std_filename = path + std_filename;
                        std::clog << "Terrain model: Loading " << std_filename << ", material num: " << i << std::endl;
                        material.texture.load(std_filename, mode, mipmap, invert);
                    }
                    catch (const std::exception& e)
                    {
                        std::clog << e.what() << std::endl;
                    }

                    break;
                }
            }

            if (filename.length == 0)
            {
                material.texture.vector() = {255, 255, 255, 255};
                material.texture.size({1, 1});
                material.texture.update();
            }
        }

        bool start = true;
        load_vertices(scene, scene->mRootNode, _M_materials, _M_limits, start);
        std::size_t triangles = 0;
        for (auto& material : _M_materials)
        {
            auto& model_mesh = material.mesh;
            model_mesh.attributes({3, 2, 3}).vertices_count(model_mesh.data().size() / 8).update_buffers();
            triangles += model_mesh.data().size() / 24;
        }

        std::clog << "Terrain model: Loaded " << triangles << " triangles" << std::endl;

        return *this;
    }

    TerrainModel::~TerrainModel()
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    TerrainModel::TerrainModel() = default;

    TerrainModel& TerrainModel::draw()
    {
        std::size_t size = _M_materials.size();
        for (std::size_t i = 0; i < size; i++)
        {
            auto& m = _M_materials[i];
            if (m.render)
            {
                m.texture.bind();
                m.mesh.draw(TRIANGLE);
            }
        }

        Texture::unbind();

        return *this;
    }

    const DrawMode& TerrainModel::mode()
    {
        return _M_mode;
    }

    TerrainModel& TerrainModel::mode(const DrawMode& mode)
    {
        _M_mode = mode;
        for (auto& material : _M_materials) material.texture.draw_mode(mode);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        return *this;
    }

    TerrainModel::TerrainModel(const std::string& model_file, const DrawMode& mode, const unsigned int& mipmap, const bool& invert)
    {
        load_model(model_file, mode, mipmap, invert);
    }

    const std::vector<TerrainModel::Material>& TerrainModel::materials() const
    {
        return _M_materials;
    }
    const TerrainModel::Limits& TerrainModel::limits() const
    {
        return _M_limits;
    }

    glm::vec<4, unsigned char, glm::defaultp> TerrainModel::default_color() const
    {
        return _M_default_color;
    }

    TerrainModel& TerrainModel::default_color(const glm::vec<4, unsigned char, glm::defaultp>& color)
    {
        _M_default_color = color;
        return *this;
    }

    bool& TerrainModel::material_render_status(const std::size_t& material_index)
    {
        if (material_index >= _M_materials.size())
            throw std::runtime_error("Terrain model::Material array: Index out of range");
        return _M_materials[material_index].render;
    }
}// namespace Engine
