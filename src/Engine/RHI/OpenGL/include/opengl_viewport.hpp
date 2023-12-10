#pragma once
#include <Graphics/rhi.hpp>

namespace Engine
{
    struct OpenGL_Viewport : public RHI_Viewport {
        WindowInterface* _M_window                   = nullptr;
        struct OpenGL_RenderTarget* _M_render_target = nullptr;


        void init(WindowInterface* window, bool vsync);
        void init(RenderTarget* render_target);

        void begin_render() override;
        void end_render() override;
        bool vsync() override;
        void vsync(bool flag) override;
        void on_resize(const Size2D& new_size) override;
        RHI_RenderTarget* render_target() override;

        ~OpenGL_Viewport();
    };
}// namespace Engine
