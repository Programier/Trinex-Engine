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
            has_depth_stancil                      = true;
            depth_stencil_attachment.clear_on_bind = true;
            depth_stencil_attachment.format        = ColorFormat::ShadowDepth;
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
        size                                         = {512, 512};
        depth_stencil_attachment.depth_stencil_clear = DepthStencilClearValue();
        depth_stencil_attachment.texture             = Object::new_instance<RenderTargetTexture>();
        depth_stencil_attachment.texture->size       = {512, 512};
        depth_stencil_attachment.texture->format     = ColorFormat::ShadowDepth;
    }

    DepthRenderTarget& DepthRenderTarget::rhi_create()
    {
        Texture2D* texture = depth_stencil_attachment.texture;
        size               = texture->size;
        depth_stencil_attachment.texture->rhi_create();
        Super::rhi_create();
        return *this;
    }
}// namespace Engine
