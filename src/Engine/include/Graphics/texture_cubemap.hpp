#pragma once

#include <Core/implement.hpp>
#include <Graphics/texture_2D.hpp>


namespace Engine
{
    class ENGINE_EXPORT TextureCubeMap : public Texture
    {
    public:
        using Super = Texture;

        TextureCubeMap();
        delete_copy_constructors(TextureCubeMap);

        TextureCubeMap& update_data(TextureCubeMapFace index, const Size2D& size, const Offset2D& offset, void* data, MipMapLevel level = 0);
    };
}// namespace Engine
