#include <Core/class.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Graphics/depth_render_target.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/render_target_texture.hpp>
#include <Graphics/texture_2D.hpp>

namespace Engine
{
    class DepthRenderPass : public EngineResource<RenderPass>
    {
        declare_class(DepthRenderPass, RenderPass);

    public:
        DepthRenderPass()
        {
            depth_stencil_attachment = ColorFormat::ShadowDepth;
        }

        RenderPassType type() const override
        {
            return RenderPassType::Depth;
        }
    };

    RenderPass* RenderPass::load_depth_render_pass()
    {
        RenderPass* pass = Object::new_non_serializable_instance<DepthRenderPass>();
        pass->init_resource(true);
        return pass;
    }

    implement_engine_class_default_init(DepthRenderPass);
    implement_engine_class_default_init(DepthRenderTarget);

    DepthRenderTarget::DepthRenderTarget()
    {
        size                             = {512, 512};
        depth_stencil_attachment         = Object::new_instance<RenderTargetTexture>();
        depth_stencil_attachment->size   = {512, 512};
        depth_stencil_attachment->format = ColorFormat::ShadowDepth;
    }

    DepthRenderTarget& DepthRenderTarget::rhi_create()
    {
        Texture2D* texture = depth_stencil_attachment;
        size               = texture->size;
        depth_stencil_attachment->rhi_create();
        Super::rhi_create();
        return *this;
    }
}// namespace Engine
