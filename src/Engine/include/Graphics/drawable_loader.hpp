#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <LibLoader/lib_loader.hpp>
#include <string>

class aiScene;

namespace Engine
{
    class DrawableObject;

    namespace ObjectLoader
    {

        namespace Assimp
        {
            ENGINE_EXPORT const aiScene* load_scene(const std::string& filename);
            ENGINE_EXPORT void close_scene(const aiScene* scene);
            ENGINE_EXPORT std::string get_error_string();
            ENGINE_EXPORT bool init();
        };// namespace Assimp

        CLASS DrawableLoader
        {
        public:
            virtual DrawableObject* load(const std::string& filename) const = 0;
        };


        CLASS TexturedObjectLoader : public DrawableLoader
        {
            virtual DrawableObject* load(const std::string& filename) const override;
        };

        CLASS PolygonalMeshLoader : public DrawableLoader
        {
            virtual DrawableObject* load(const std::string& filename) const override;
        };
    }// namespace ObjectLoader
}// namespace Engine
