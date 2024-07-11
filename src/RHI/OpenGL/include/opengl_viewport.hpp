#pragma once
#include <Graphics/rhi.hpp>

namespace Engine
{
    class Window;
    struct OpenGL_Viewport : public RHI_DefaultDestroyable<RHI_Viewport> {
        void vsync(bool flag) override;
        void on_resize(const Size2D& new_size) override;
    };

    struct OpenGL_WindowViewport : OpenGL_Viewport {
        RenderViewport* m_viewport = nullptr;

        void begin_render() override;
        void end_render() override;

        void init(RenderViewport* viewport);
        void vsync(bool flag) override;
        void make_current();
        static OpenGL_WindowViewport* current();
        void bind() override;
        void blit_target(RenderSurface* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override;
        void clear_color(const Color& color) override;
        ~OpenGL_WindowViewport();
    };

}// namespace Engine
