#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/texture_cubemap.hpp>

namespace Engine
{
    implement_class(TextureCubeMap, "Engine");
    implement_default_initialize_class(TextureCubeMap);

    TextureCubeMap::TextureCubeMap() = default;

    TextureType TextureCubeMap::type() const
    {
        return TextureType::TextureCubeMap;
    }

    TextureCubeMap& TextureCubeMap::rhi_create()
    {
        rhi_destroy();
        _M_rhi_texture = engine_instance->rhi()->create_texture(this, TextureType::TextureCubeMap, nullptr);
        return *this;
    }

    TextureCubeMap& TextureCubeMap::update_data(TextureCubeMapFace index, const Size2D& size, const Offset2D& offset,
                                                void* data, MipMapLevel level)
    {
        //        EngineInstance::instance()->api_interface()->cubemap_texture_update_data(_M_ID, index, size, offset, level,
        //                                                                                 data);
        return *this;
    }
}// namespace Engine
