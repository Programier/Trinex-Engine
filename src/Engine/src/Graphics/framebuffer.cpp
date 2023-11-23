#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/render_target.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    implement_class(RenderTarget, "Engine");
    implement_default_initialize_class(RenderTarget);

    RenderTarget& RenderTarget::rhi_create()
    {
        Super::rhi_create();
        _M_rhi_render_target = EngineInstance::instance()->rhi()->create_render_target(this);
        return *this;
    }
}// namespace Engine
