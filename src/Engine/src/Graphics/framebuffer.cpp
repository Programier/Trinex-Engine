#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/framebuffer.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    implement_class(RenderTarget, "Engine");
    implement_default_initialize_class(RenderTarget);

    RenderTarget& RenderTarget::rhi_create()
    {
        ApiObject::destroy();
        _M_rhi_render_target = EngineInstance::instance()->api_interface()->create_render_target(this);
        return *this;
    }
}// namespace Engine
