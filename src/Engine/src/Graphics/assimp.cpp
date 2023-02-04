#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/assimp.hpp>
#include <LibLoader/lib_loader.hpp>
#include <assimp/cimport.h>
#include <assimp/material.h>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Engine
{
    namespace AssimpLibrary
    {
        static Library assimp;
        static const C_STRUCT aiScene* (*assimp_load_scene)(const char*, unsigned int) = nullptr;
        static const char* (*assimp_get_error)() = nullptr;
        static void (*assimp_close_scene)(const C_STRUCT aiScene*) = nullptr;
        static aiReturn (*assimp_get_material_string)(const aiMaterial*, const char*, unsigned int, unsigned int,
                                                      aiString*) = nullptr;


        static void init_funcs()
        {
            if (!assimp.has_lib())
                return;

            assimp_load_scene =
                    assimp.get<const C_STRUCT aiScene*, const char*, unsigned int>(lib_function(aiImportFile));
            assimp_get_error = assimp.get<const char*>(lib_function(aiGetErrorString));
            assimp_close_scene = assimp.get<void, const C_STRUCT aiScene*>(lib_function(aiReleaseImport));
            assimp_get_material_string =
                    assimp.get<aiReturn, const aiMaterial*, const char*, unsigned int, unsigned int, aiString*>(
                            lib_function(aiGetMaterialString));
        }

        ENGINE_EXPORT const aiScene* load_scene(const String& filename)
        {
            AssimpLibrary::init();
            if (!assimp_load_scene)
                return nullptr;
            auto scene = assimp_load_scene(filename.c_str(), aiProcess_Triangulate | aiProcess_GenNormals |
                                                                     aiProcess_GenBoundingBoxes | aiProcess_FlipUVs |
                                                                     aiProcess_PopulateArmatureData);

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

        ENGINE_EXPORT Library& get_library()
        {
            return assimp;
        }

        ENGINE_EXPORT std::string get_material_string(const aiMaterial* material, const char* a, unsigned int b,
                                                      unsigned int c)
        {
            if (assimp_get_material_string)
            {
                aiString result;
                assimp_get_material_string(material, a, b, c, &result);
                return result.data;
            }

            return {};
        }


    };// namespace AssimpLibrary
}// namespace Engine
