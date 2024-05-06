#pragma once
#include <Core/enums.hpp>
#include <Core/render_resource.hpp>

namespace Engine
{
    class Texture2D;

    class ENGINE_EXPORT RenderPass : public RenderResource
    {
        declare_class(RenderPass, RenderResource);

    private:
        static RenderPass* load_window_render_pass();
        static RenderPass* load_one_attachement_render_pass();
        static RenderPass* load_gbuffer_render_pass();
        static RenderPass* load_depth_render_pass();

    public:
        Vector<ColorFormat> color_attachments;
        ColorFormat depth_stencil_attachment = ColorFormat::Undefined;

        RenderPass();
        RenderPass& rhi_create() override;
        virtual RenderPassType type() const;

        static RenderPass* load_render_pass(RenderPassType type);
    };
}// namespace Engine
