#include <Graphics/texture_cubemap.hpp>
#include <api_funcs.hpp>

namespace Engine
{
    declare_instance_info_cpp(TextureCubeMap);
    constructor_cpp(TextureCubeMap)
    {}

    TextureCubeMap& TextureCubeMap::attach_texture(const Texture2D& texture, TextureCubeMapFace index, int level)
    {
        cubemap_texture_attach_2d_texture(_M_ID, texture.id(), index, level);
        return *this;
    }

    TextureCubeMap& TextureCubeMap::attach_data(TextureCubeMapFace index, const Size2D& size, void* data, int level)
    {
        cubemap_texture_attach_data(_M_ID, index, size, level, data);
        return *this;
    }
}// namespace Engine
