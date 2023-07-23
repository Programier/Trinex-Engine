#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/texture_cubemap.hpp>
#include <api.hpp>

namespace Engine
{
    static void on_init()
    {
        register_class(Engine::TextureCubeMap);
    }

    static InitializeController initializer(on_init);

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
