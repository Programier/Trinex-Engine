#pragma once

#include <Core/implement.hpp>
#include <Graphics/texture_2D.hpp>


namespace Engine
{
    CLASS TextureCubeMap : public Texture
    {
        Texture2D _M_textures[6];

        declare_instance_info_hpp(TextureCubeMap);

    public:
        constructor_hpp(TextureCubeMap);
        delete_copy_constructors(TextureCubeMap);

        TextureCubeMap& attach_texture(const Texture2D& texture, TextureCubeMapFace index, int level = 0);
        TextureCubeMap& attach_data(TextureCubeMapFace index, const Size2D& size, void* data, int level = 0);
    };
}// namespace Engine
