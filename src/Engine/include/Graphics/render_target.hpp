#pragma once
#include <Core/implement.hpp>
#include <Graphics/render_target_base.hpp>
#include <Graphics/texture_2D.hpp>


namespace Engine
{
    class Texture2D;

    class ENGINE_EXPORT RenderTarget : public RenderTargetBase
    {
        declare_class(RenderTarget, RenderTargetBase);

    public:
        struct Frame {
            Vector<Pointer<Texture2D>> color_attachments;
            Pointer<Texture2D> depth_stencil_attachment;

            virtual ~Frame();
        };

    protected:
        Vector<Frame*> m_frames;

        RenderTarget& push_frame(Frame* frame);
        RenderTarget& clear_frames(bool with_delete = true);

    public:
        Vector<ColorClearValue> color_clear;
        DepthStencilClearValue depth_stencil_clear;
        Size2D size;


        RenderTarget();
        RenderTarget& rhi_create() override;
        Frame* current_frame() const;
        Frame* frame(byte index) const;
        size_t frames_count() const;
        Size2D render_target_size() const override;

        ~RenderTarget() override;
    };
}// namespace Engine
