#include <Core/class.hpp>
#include <Graphics/render_pass.hpp>
#include <Core/engine.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    implement_class(RenderPass, "Engine");
    implement_default_initialize_class(RenderPass);

    Vector<RenderPass*> RenderPass::_M_default_render_passes(RenderPass::Type::__COUNT__, nullptr);

    RenderPass* RenderPass::default_pass(RenderPass::Type type)
    {
        Index index = static_cast<Index>(type);
        return _M_default_render_passes.size() <= index ? nullptr : _M_default_render_passes[index];
    }

    RenderPass& RenderPass::rhi_create()
    {
        rhi_destroy();
        _M_rhi_object = engine_instance->rhi()->create_render_pass(this);
        return *this;
    }
}// namespace Engine
