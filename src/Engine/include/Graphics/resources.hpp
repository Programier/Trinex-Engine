#pragma once
#include <vector>
#include <Core/export.hpp>

namespace Engine
{
    class BasicMesh;
    class Texture;
    class Drawable;

    namespace Resources
    {
        ENGINE_EXPORT extern std::vector<BasicMesh*> meshes;
        ENGINE_EXPORT extern std::vector<Texture*> textures;
        ENGINE_EXPORT extern std::vector<Drawable*> drawables;
    }

}// namespace Engine
