#pragma once
#include <Graphics/rhi.hpp>

namespace Engine
{
    struct OpenGL_Viewport : public RHI_Viewport {
        struct OpenGL_RenderTarget* _M_render_target = nullptr;

        bool vsync() override;
        void vsync(bool flag) override;
        void on_resize(const Size2D& new_size) override;
        RHI_RenderTarget* render_target() override;
    };

    struct OpenGL_RenderTargetViewport : public OpenGL_Viewport {
        void init(RenderTarget* render_target);

        void begin_render() override;
        void end_render() override;
    };

    struct OpenGL_WindowViewport : OpenGL_Viewport {
        void* _M_context           = nullptr;
        WindowInterface* _M_window = nullptr;

        void begin_render() override;
        void end_render() override;

        void init(WindowInterface* window, bool vsync);
        bool vsync() override;
        void vsync(bool flag) override;

        ~OpenGL_WindowViewport();
    };

}// namespace Engine
