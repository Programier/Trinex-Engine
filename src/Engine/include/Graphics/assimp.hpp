#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <LibLoader/lib_loader.hpp>
#include <string>

class aiScene;
class aiMaterial;

namespace Engine
{
    namespace AssimpLibrary
    {
        ENGINE_EXPORT const aiScene* load_scene(const std::string& filename);
        ENGINE_EXPORT void close_scene(const aiScene* scene);
        ENGINE_EXPORT std::string get_error_string();
        ENGINE_EXPORT bool init();
        ENGINE_EXPORT Library& get_library();
        ENGINE_EXPORT std::string get_material_string(const aiMaterial* material, const char*, unsigned int, unsigned int);
    };// namespace Assimp
}// namespace Engine
