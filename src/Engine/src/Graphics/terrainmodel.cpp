#include <Graphics/terrainmodel.hpp>
#include <LibLoader/lib_loader.hpp>
#include <SDL_log.h>
#include <fstream>
#include <model_loader.hpp>
#include <opengl.hpp>
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


#define check_vert(v)                                                                                                       \
    if (_M_limits.min.v > vert.v)                                                                                           \
    {                                                                                                                       \
        _M_limits.min.v = vert.v;                                                                                           \
    }                                                                                                                       \
    if (_M_limits.max.v < vert.v)                                                                                           \
    {                                                                                                                       \
        _M_limits.max.v = vert.v;                                                                                           \
    }

    static void load_vertices(const aiScene* scene, aiNode* node, std::vector<TerrainModel::Material>& materials,
                              TerrainModel::Limits& _M_limits, bool& start, glm::mat4 matrix = identity_matrix)
    {
        matrix *= mat4(node->mTransformation.Transpose());
        glm::mat3 normal_matrix = glm::transpose(glm::inverse(matrix));
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
                        glm::vec3 normal = glm::normalize(normal_matrix * vec3(ai_mesh->mNormals[face.mIndices[index]]));
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

        Library assimp = load_library("assimp");
        if (!assimp.has_lib())
            return *this;


        auto assimp_ReleaseImport = assimp.get<void, const C_STRUCT aiScene*>(lib_function(aiReleaseImport));

        auto assimp_mat_texture = assimp.get<aiReturn, const C_STRUCT aiMaterial*, aiTextureType, unsigned int, aiString*>(
                lib_function(aiGetMaterialTexture));

        //        auto assimp_material_name = assimp.get();


        auto scene = load_scene(model_file);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            SDL_Log("Terrain loader: Failed to load %s\n", model_file.c_str());
            assimp_ReleaseImport(scene);
            return *this;
        }

        SDL_Log("Terraing model: Material num: %u\n", scene->mNumMaterials);
        _M_materials.resize(scene->mNumMaterials);

        // Loading materials
        for (unsigned int i = 0; i < scene->mNumMaterials; i++)
        {
            auto& ai_material = scene->mMaterials[i];
            auto& material = _M_materials[i];
            material.index = i;
            // material.name = ai_material->GetName().C_Str();

            aiString filename;
            assimp_mat_texture(ai_material, aiTextureType_DIFFUSE, 0, &filename);
            std::string texture_file(filename.C_Str());
            auto& texture = _M_textures[texture_file];

            if (texture.texture.empty())
            {
                try
                {
                    texture_file = path + texture_file;
                    SDL_Log("Terrain model: Loading %s, material num: %u\n", texture_file.c_str(), i);
                    texture.texture.load(texture_file, mode, mipmap, invert);
                }
                catch (const std::exception& e)
                {
                    SDL_Log("%s\n", e.what());
                    texture.texture.vector() = {255, 255, 255, 255};
                    texture.texture.size({1, 1});
                    texture.texture.update();
                    texture.default_texture = true;
                }
            }
            material.texture = ReferenceWrapper(texture.texture);
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


        SDL_Log("Terrain model: Loaded %zu triangles\n", triangles);
        SDL_Log("Terrain model: Materials count %zu\n", _M_materials.size());
        SDL_Log("Terrain model: Textures count %zu\n", _M_textures.size());
        assimp_ReleaseImport(scene);
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
                m.texture.get().bind();
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
        for (auto& key : _M_textures) key.second.texture.draw_mode(mode);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        return *this;
    }

    TerrainModel::TerrainModel(const std::string& model_file, const DrawMode& mode, const unsigned int& mipmap,
                               const bool& invert)
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

    Color TerrainModel::default_color() const
    {
        return _M_default_color;
    }

    TerrainModel& TerrainModel::default_color(const Color& color)
    {
        //_M_default_color = color;
        for (auto& value : _M_textures)
        {
            auto& texture = value.second;
            if (texture.default_texture)
            {
                texture.texture.vector() = {cast(byte, color.r * 255), cast(byte, color.g * 255), cast(byte, color.b * 255),
                                            cast(byte, color.a * 255)};
                texture.texture.size({1, 1});
                texture.texture.update();
            }
        }
        return *this;
    }

    bool& TerrainModel::material_render_status(const std::size_t& material_index)
    {
        if (material_index >= _M_materials.size())
            throw std::runtime_error("Terrain model::Material array: Index out of range");
        return _M_materials[material_index].render;
    }
}// namespace Engine
