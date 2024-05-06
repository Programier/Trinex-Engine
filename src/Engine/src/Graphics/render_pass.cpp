#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    implement_class(RenderPass, Engine, 0);
    implement_default_initialize_class(RenderPass);


    RenderPass::RenderPass()
    {
        flags(IsSerializable, false);
    }

    RenderPass& RenderPass::rhi_create()
    {
        m_rhi_object.reset(engine_instance->rhi()->create_render_pass(this));
        return *this;
    }


    static RenderPass* render_passes[static_cast<EnumerateType>(RenderPassType::__COUNT__)] = {};

    RenderPass* RenderPass::load_render_pass(RenderPassType type)
    {
        RenderPass*& pass = render_passes[static_cast<EnumerateType>(type)];
        if (pass)
            return pass;

        switch (type)
        {
            case RenderPassType::Window:
                pass = load_window_render_pass();
                break;

            case RenderPassType::OneAttachentOutput:
                pass = load_one_attachement_render_pass();
                break;
            case RenderPassType::GBuffer:
                pass = load_gbuffer_render_pass();
                break;

            case RenderPassType::Depth:
                pass = load_depth_render_pass();
                break;

            default:
                break;
        }

        return pass;
    }

    RenderPassType RenderPass::type() const
    {
        return RenderPassType::Undefined;
    }
}// namespace Engine
