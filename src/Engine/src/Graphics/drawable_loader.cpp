#include <Core/logger.hpp>
#include <Core/string_convert.hpp>
#include <Core/string_format.hpp>
#include <Graphics/drawable_loader.hpp>
#include <Graphics/line.hpp>
#include <Graphics/resources.hpp>
#include <Graphics/textured_object.hpp>
#include <Image/image.hpp>
#include <LibLoader/lib_loader.hpp>
#include <assimp/cimport.h>
#include <assimp/material.h>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stack>

namespace Engine::ObjectLoader
{

    static const glm::mat4 aiMatrix_to_matrix(const aiMatrix4x4& matrix)
    {
        glm::mat4 out;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                out[i][j] = matrix[i][j];
            }
        }
        return out;
    }

    static glm::vec3 aiVec3_to_vec3(const aiVector3D& vector)
    {
        return glm::vec3(vector.x, vector.y, vector.z);
    }

#define assimp_material_filename_prototype                                                                                  \
    const C_STRUCT aiMaterial *, aiTextureType, unsigned int, aiString *, aiTextureMapping *, unsigned int *, ai_real *,    \
            aiTextureOp *, aiTextureMapMode *, unsigned int *


    namespace Assimp
    {
        static Library assimp;
        static const C_STRUCT aiScene* (*assimp_load_scene)(const char*, unsigned int) = nullptr;
        static const char* (*assimp_get_error)() = nullptr;
        static void (*assimp_close_scene)(const C_STRUCT aiScene*) = nullptr;
        static aiReturn (*assimp_get_material_string)(const aiMaterial*, const char*, unsigned int, unsigned int,
                                                      aiString*) = nullptr;
        static aiReturn (*assimp_get_material_filename)(assimp_material_filename_prototype) = nullptr;

        static void init_funcs()
        {
            if (!assimp.has_lib())
                return;

            assimp_load_scene = assimp.get<const C_STRUCT aiScene*, const char*, unsigned int>(lib_function(aiImportFile));
            assimp_get_error = assimp.get<const char*>(lib_function(aiGetErrorString));
            assimp_close_scene = assimp.get<void, const C_STRUCT aiScene*>(lib_function(aiReleaseImport));
            assimp_get_material_string =
                    assimp.get<aiReturn, const aiMaterial*, const char*, unsigned int, unsigned int, aiString*>(
                            lib_function(aiGetMaterialString));

            assimp_get_material_filename =
                    assimp.get<aiReturn, assimp_material_filename_prototype>(lib_function(aiGetMaterialTexture));
        }

        ENGINE_EXPORT const aiScene* load_scene(const std::string& filename)
        {
            if (!assimp_load_scene)
                return nullptr;
            auto scene = assimp_load_scene(filename.c_str(), aiProcess_Triangulate | aiProcess_GenNormals |
                                                                     aiProcess_GenBoundingBoxes | aiProcess_ForceGenNormals |
                                                                     aiProcess_SplitLargeMeshes | aiProcess_RemoveComponent);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            {
                logger->log("Model loader: %s\n", get_error_string().c_str());

                if (scene)
                    close_scene(scene);
                return nullptr;
            }

            return scene;
        }

        ENGINE_EXPORT void close_scene(const aiScene* scene)
        {
            if (assimp_close_scene)
                assimp_close_scene(scene);
        }

        ENGINE_EXPORT std::string get_error_string()
        {
            if (assimp_get_error)
                return assimp_get_error();
            return "Assimp loader error";
        }

        ENGINE_EXPORT bool init()
        {
            if (!assimp.has_lib())
            {
                assimp = load_library("assimp");
                init_funcs();
            }
            return assimp.has_lib();
        }


    };// namespace Assimp


    static const aiScene* load_scene_from_file(const std::string& filename)
    {
        if (!Assimp::init())
        {
            logger->log("Failed to init assimp\n");
            return nullptr;
        }
        const aiScene* scene = Assimp::load_scene(filename);

        if (!scene)
            logger->log("Loader error: %s\n", Assimp::get_error_string().c_str());

        return scene;
    }


    template<typename Type>
    Type* for_each_ainode(const std::string& filename,
                          void (*callback)(const aiScene*, aiNode*, Type* object, const std::string&),
                          const aiScene* (*scene_loader)(const std::string&) = load_scene_from_file,
                          const std::string& dirname = "")
    {
        const aiScene* scene = scene_loader(filename);
        if (!scene)
            return nullptr;

        struct Node {
            Type* engine_object = nullptr;
            aiNode* object_node = nullptr;

            Type* prev = nullptr;

            Node(Type* eo, aiNode* on, Type* prev_node = nullptr)
            {
                engine_object = eo;
                object_node = on;
                prev = prev_node;
            }
        };

        Type* head = new Type();

        std::stack<Node> _M_nodes;
        _M_nodes.push(Node(head, scene->mRootNode));


        while (!_M_nodes.empty())
        {
            Node node = _M_nodes.top();
            _M_nodes.pop();

            node.engine_object->name(Strings::to_wstring(node.object_node->mName.C_Str()));
            logger->log("Loading node '%s'\n", node.object_node->mName.C_Str());
            auto transform = aiMatrix_to_matrix(node.object_node->mTransformation.Transpose());
            node.engine_object->model(transform);


            callback(scene, node.object_node, node.engine_object, dirname);

            if (node.prev)
            {
                node.prev->push_object(node.engine_object);
            }

            for (unsigned int i = 0; i < node.object_node->mNumChildren; i++)
            {
                Type* new_node = new Type();
                Node tmp(new_node, node.object_node->mChildren[i], node.engine_object);
                _M_nodes.push(tmp);
            }
        }

        Assimp::close_scene(scene);
        return head;
    }


    /////////////////////// POLYGONAL MESH OBJECT ///////////////////////


    Line* procces_polygonal_mesh(DrawableObject* root, aiMesh* mesh, unsigned int mesh_index)
    {
        AABB_3D aabb;
        aabb.min = aiVec3_to_vec3(mesh->mAABB.mMin);
        aabb.max = aiVec3_to_vec3(mesh->mAABB.mMax);


        Line* line = new Line();
        line->name(root->name() + std::wstring(L".Mesh_") + std::to_wstring(mesh_index));
        line->aabb(aabb);
        root->push_object(line);

        std::unordered_map<unsigned int, unsigned int> _M_points;

        for (unsigned int f = 0; f < mesh->mNumFaces; f++)
        {
            auto& face = mesh->mFaces[f];
            for (unsigned int index = 0; index < face.mNumIndices; index++)
            {

                if (!_M_points.contains(face.mIndices[index]))
                {
                    auto begin = aiVec3_to_vec3(mesh->mVertices[face.mIndices[index]]);

                    _M_points.insert({face.mIndices[index], line->data.size() / 3});

                    line->data.push_back(begin.x);
                    line->data.push_back(begin.y);
                    line->data.push_back(begin.z);
                }


                for (unsigned int g = index + 1; g < face.mNumIndices; g++)
                {
                    line->indexes.push_back(_M_points.at(face.mIndices[index]));

                    if (_M_points.contains(face.mIndices[g]))
                    {
                        line->indexes.push_back(_M_points.at(face.mIndices[g]));
                    }
                    else
                    {
                        line->indexes.push_back(line->data.size() / 3);
                        _M_points.insert({face.mIndices[g], line->data.size() / 3});

                        auto end = aiVec3_to_vec3(mesh->mVertices[face.mIndices[g]]);

                        line->data.push_back(end.x);
                        line->data.push_back(end.y);
                        line->data.push_back(end.z);
                    }
                }
            }
        }

        line->update();
        return line;
    }

    void polygonal_object_load_callback(const aiScene* scene, aiNode* ainode, Line* object, const std::string&)
    {
        for (unsigned int i = 0; i < ainode->mNumMeshes; i++)
        {
            auto& mesh = scene->mMeshes[ainode->mMeshes[i]];
            procces_polygonal_mesh(object, mesh, i + 1);
        }
    }

    DrawableObject* PolygonalMeshLoader::load(const std::string& filename) const
    {
        logger->log("PolygonalMesh loader: Start loading '%s'\n", filename.c_str());
        Line* head = for_each_ainode<Line>(filename, polygonal_object_load_callback);

        if (head)
            logger->log("PolygonalMesh loader: Loading the \"%s\" model completed successfully\n", filename.c_str());

        return head;
    }

    /////////////////////// TEXTURED OBJECT ///////////////////////

    struct TextureNode {
        Texture2D _M_diffuse;
    };

    static std::unordered_map<unsigned int, TextureNode> _M_texture_pair;

    static void init_object(TexturedObject* object)
    {
        object->attributes = {{3, BufferValueType::FLOAT}, {2, BufferValueType::FLOAT}, {3, BufferValueType::FLOAT}};
        object->vertices = object->indexes.size();
        if (object->Mesh::id() == 0)
        {
            object->Mesh::mode = DrawMode::STATIC_DRAW;
            object->Mesh::gen();
        }

        object->set_data();
        object->update_atributes().update_indexes();
    }

    static Texture2D& get_texture(const std::string& dir, unsigned int index, const aiMaterial* material,
                                  const aiTextureType& type, Texture2D& (*get_texture_from_node)(TextureNode&) )
    {
        if (_M_texture_pair.contains(index))
        {
            return get_texture_from_node(_M_texture_pair.at(index));
        }


        std::string filename;
        if (Assimp::assimp_get_material_filename)
        {
            aiString _file;
            Assimp::assimp_get_material_filename(material, aiTextureType::aiTextureType_DIFFUSE, 0, &_file, nullptr, nullptr,
                                                 nullptr, nullptr, nullptr, nullptr);
            filename = _file.C_Str();
        }

        _M_texture_pair.insert_or_assign(index, TextureNode());

        Texture2D& texture = get_texture_from_node(_M_texture_pair.at(index));

        logger->log("Loading Texture '%s'\n", filename.c_str());

        Image image(dir + filename, true);
        TextureParams params;

        static PixelFormat _M_formats[5] = {PixelFormat::RGBA, PixelFormat::DEPTH, PixelFormat::DEPTH, PixelFormat::RGB,
                                            PixelFormat::RGBA};
        params.format = _M_formats[static_cast<int>(image.channels())];

        params.pixel_type = BufferValueType::UNSIGNED_BYTE;
        params.type = TextureType::Texture_2D;
        params.border = false;


        texture.create(params);
        if (image.empty())
        {
            std::vector<byte> tmp = {100, 100, 100, 255};
            texture.gen({1, 1}, 0, (void*) tmp.data());
        }
        else
        {
            logger->log("Image data: %zu, {%f, %f}\n", image.vector().size(), image.size().x, image.size().y);
            texture.gen(image.size(), 0, (void*) image.vector().data()).max_mipmap_level(2).generate_mipmap();
        }

        texture.min_filter(TextureFilter::LINEAR).mag_filter(TextureFilter::LINEAR);

        return texture;
    }

    static Texture2D& get_diffuse_from_texture_node(TextureNode& node)
    {
        return node._M_diffuse;
    }

    void textured_object_load_callback(const aiScene* scene, aiNode* ainode, TexturedObject* object, const std::string& dir)
    {
        for (unsigned int i = 0; i < ainode->mNumMeshes; i++)
        {
            auto& mesh = scene->mMeshes[ainode->mMeshes[i]];

            if (mesh->mPrimitiveTypes == aiPrimitiveType_LINE)
            {
                procces_polygonal_mesh(object, mesh, i + 1);
                continue;
            }

            AABB_3D aabb;
            aabb.min = aiVec3_to_vec3(mesh->mAABB.mMin);
            aabb.max = aiVec3_to_vec3(mesh->mAABB.mMax);

            TexturedObject* node = new TexturedObject();
            node->name(object->name() + std::wstring(L".Mesh_") + std::to_wstring(i));
            node->aabb(aabb);
            object->push_object(node);


            auto material = scene->mMaterials[mesh->mMaterialIndex];

            // Try to set texture

            node->diffuse_texture(
                    get_texture(dir, mesh->mMaterialIndex, material, aiTextureType_DIFFUSE, get_diffuse_from_texture_node));


            std::unordered_map<unsigned int, unsigned int> _M_vertices;

            for (unsigned int f = 0; f < mesh->mNumFaces; f++)
            {
                auto& face = mesh->mFaces[f];
                for (unsigned int index = 0; index < face.mNumIndices; index++)
                {
                    if (_M_vertices.contains(face.mIndices[index]))
                    {
                        node->indexes.push_back(_M_vertices.at(face.mIndices[index]));
                    }
                    else
                    {
                        node->indexes.push_back(node->data.size() / 8);
                        _M_vertices.insert({face.mIndices[index], node->data.size() / 8});

                        auto& face_point = mesh->mVertices[face.mIndices[index]];
                        Vector2D UV = mesh->mTextureCoords[0] ? aiVec3_to_vec3(mesh->mTextureCoords[0][face.mIndices[index]])
                                                              : Vector2D{0.f, 0.f};
                        //auto& normal = mesh->mNormals[face.mIndices[index]];

                        // Coords
                        node->data.push_back(face_point.x);
                        node->data.push_back(face_point.y);
                        node->data.push_back(face_point.z);

                        // UV
                        node->data.push_back(UV.x);
                        node->data.push_back(UV.y);

                        // Normals
                        node->data.push_back(0);
                        node->data.push_back(0);
                        node->data.push_back(0);
                    }
                }
            }

            init_object(node);
        }
    }

    static std::string dirname_of(const std::string& fname)
    {
        size_t pos = fname.find_last_of("\\/");
        return (std::string::npos == pos) ? "./" : fname.substr(0, pos + 1);
    }


    DrawableObject* TexturedObjectLoader::load(const std::string& filename) const
    {
        _M_texture_pair.clear();
        logger->log("TexturedObject loader: Start loading '%s'\n", filename.c_str());

        TexturedObject* head = for_each_ainode<TexturedObject>(filename, textured_object_load_callback, load_scene_from_file,
                                                               dirname_of(filename));

        if (head)
            logger->log("TexturedObject loader: Loading the \"%s\" model completed successfully\n", filename.c_str());

        for (auto& ell : _M_texture_pair) Resources::push_texture(ell.second._M_diffuse);

        _M_texture_pair.clear();
        return head;
    }

}// namespace Engine::ObjectLoader
