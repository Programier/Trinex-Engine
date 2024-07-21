#pragma once
#include <Graphics/rhi.hpp>
#include <d3d11.h>

namespace Engine
{
    class D3D11_WindowViewport : public RHI_DefaultDestroyable<RHI_Viewport>
    {
    public:
        IDXGISwapChain* m_swap_chain   = nullptr;
        ID3D11Texture2D* m_back_buffer = nullptr;
        ID3D11RenderTargetView* m_view = nullptr;
        class Window* m_window         = nullptr;
        Size2D m_size                  = {0.f, 0.f};
        bool m_with_vsync              = false;

        void init(class Window* window);
        void create_swapchain(const Size2D& size);

        void begin_render() override;
        void end_render() override;
        void vsync(bool flag) override;
        void on_resize(const Size2D& new_size) override;
        void bind() override;
        void blit_target(RenderSurface* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
                         SamplerFilter filter) override;
        void clear_color(const Color& color) override;

        ~D3D11_WindowViewport();
    };
}// namespace Engine
