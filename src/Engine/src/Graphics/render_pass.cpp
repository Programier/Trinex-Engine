#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    implement_class(RenderPass, Engine, 0);
    implement_default_initialize_class(RenderPass);

    RenderPass& RenderPass::rhi_create()
    {
        _M_rhi_object.reset(engine_instance->rhi()->create_render_pass(this));
        return *this;
    }
}// namespace Engine
