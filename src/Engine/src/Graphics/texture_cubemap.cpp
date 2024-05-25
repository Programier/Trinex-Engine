#include <Core/class.hpp>
#include <Core/base_engine.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/texture_cubemap.hpp>

namespace Engine
{
    implement_class(TextureCubeMap, Engine, 0);
    implement_default_initialize_class(TextureCubeMap);

    TextureCubeMap::TextureCubeMap() = default;

    TextureType TextureCubeMap::type() const
    {
        return TextureType::TextureCubeMap;
    }

    TextureCubeMap& TextureCubeMap::rhi_create()
    {
        m_rhi_object.reset(rhi->create_texture(this, nullptr, 0));
        return *this;
    }

    TextureCubeMap& TextureCubeMap::update_data(TextureCubeMapFace index, const Size2D& size, const Offset2D& offset,
                                                void* data, MipMapLevel level)
    {
        //        EngineInstance::instance()->api_interface()->cubemap_texture_update_data(m_ID, index, size, offset, level,
        //                                                                                 data);
        return *this;
    }
}// namespace Engine
