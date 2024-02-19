#pragma once
#include <Core/color_format.hpp>
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
        static RenderPass* load_scene_color_render_pass();
        static RenderPass* load_gbuffer_render_pass();

        static RenderPass* load_clear_scene_color_render_pass();
        static RenderPass* load_clear_gbuffer_render_pass();

    public:
        struct Attachment {
            ColorFormat format;
            bool clear_on_bind = true;
        };

        Vector<Attachment> color_attachments;
        Attachment depth_stencil_attachment;
        bool has_depth_stancil = false;

        RenderPass();
        RenderPass& rhi_create() override;
        virtual RenderPassType type() const;

        static RenderPass* load_render_pass(RenderPassType type);
    };
}// namespace Engine
