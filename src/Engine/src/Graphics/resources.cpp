#include <Core/destroy_controller.hpp>
#include <Core/logger.hpp>
#include <Graphics/drawable.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/resources.hpp>
#include <Graphics/texture.hpp>

namespace Engine::Resources
{
    ENGINE_EXPORT std::vector<BasicMesh*> meshes;
    ENGINE_EXPORT std::vector<Texture*> textures;
    ENGINE_EXPORT std::vector<Drawable*> drawables;


    //    DestroyController({
    //        logger->log("Engine: Start delete resources\n");
    //        for (auto mesh : meshes) delete mesh;
    //        for (auto texture : textures) delete texture;
    //        for (auto drawable : drawables) delete drawable;
    //    });
}// namespace Engine::Resources
