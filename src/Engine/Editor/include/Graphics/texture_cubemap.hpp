#pragma once

#include <Graphics/texture_2D.hpp>
#include <Core/implement.hpp>


namespace Engine
{
    CLASS TextureCubeMap : public Texture
    {
        Texture2D _M_textures[6];

    public:
        implement_class_hpp(TextureCubeMap);
        TextureCubeMap& attach_texture(const Texture2D& texture, TextureCubeMapFace index, int level = 0);
        TextureCubeMap& attach_data(TextureCubeMapFace index, const Size2D& size, void* data, int level = 0);
    };
}
