#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Graphics/render_target_texture.hpp>
#include <Graphics/rhi.hpp>


namespace Engine
{
    implement_engine_class_default_init(RenderTargetTexture);

    RenderTargetTexture::RenderTargetTexture()
    {
        flags(IsSerializable, false);
        flags(IsEditable, false);
    }

    bool RenderTargetTexture::is_render_target_texture() const
    {
        return true;
    }

}// namespace Engine
