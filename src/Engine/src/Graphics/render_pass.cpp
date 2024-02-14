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
        _M_rhi_object.reset(engine_instance->rhi()->create_render_pass(this));
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

            case RenderPassType::SceneOutput:
                pass = load_scene_color_render_pass();
                break;
            case RenderPassType::GBuffer:
                pass = load_gbuffer_render_pass();
                break;
            default:
                break;
        }

        if(pass)
        {
            pass->add_reference();
        }
        return pass;
    }

    RenderPassType RenderPass::type() const
    {
        return RenderPassType::Undefined;
    }
}// namespace Engine
