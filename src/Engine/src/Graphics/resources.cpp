#include <Graphics/resources.hpp>

namespace Engine
{
    static struct {
        ResouceMap<Texture> _M_textures;
    } _M_resources_data;

    static Resources _M_resources;

    ENGINE_EXPORT const ResouceMap<Texture>& Resources::textures()
    {
        return _M_resources_data._M_textures;
    }

    const Resources& Resources::push_texture(const Texture& texture)
    {
        _M_resources_data._M_textures.insert_or_assign(texture.id(), texture);
        return _M_resources;
    }

    const Resources& Resources::remove_texture(const Texture& texture)
    {
        _M_resources_data._M_textures.erase(texture.id());
        return _M_resources;
    }

}// namespace Engine
