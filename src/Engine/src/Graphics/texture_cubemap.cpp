#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/texture_cubemap.hpp>
#include <api.hpp>

namespace Engine
{
    implement_class(TextureCubeMap, "Engine");
    implement_default_initialize_class(TextureCubeMap);

    TextureCubeMap::TextureCubeMap()
    {
        _M_type = TextureType::TextureCubeMap;
    }

    TextureCubeMap& TextureCubeMap::update_data(TextureCubeMapFace index, const Size2D& size, const Offset2D& offset,
                                                void* data, MipMapLevel level)
    {
        EngineInstance::instance()->api_interface()->cubemap_texture_update_data(_M_ID, index, size, offset, level,
                                                                                 data);
        return *this;
    }
}// namespace Engine
