#pragma once
#include <Graphics/rhi.hpp>
#include <d3d11.h>

namespace Engine
{
	class D3D11_Viewport : public RHI_DefaultDestroyable<RHI_Viewport>
	{
	public:
		virtual bool is_window_viewport() const               = 0;
		virtual Size2D render_target_size() const             = 0;
		virtual ID3D11RenderTargetView* render_target() const = 0;

		void bind() override;
		void vsync(bool flag) override;
		void on_orientation_changed(Orientation orientation) override;
		void on_resize(const Size2D& new_size) override;
		void clear_color(const Color& color) override;
		void blit_target(RHI_RenderTargetView* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
						 SamplerFilter filter) override;
	};

	class D3D11_WindowViewport : public D3D11_Viewport
	{
	public:
		IDXGISwapChain* m_swap_chain   = nullptr;
		ID3D11Texture2D* m_back_buffer = nullptr;
		ID3D11RenderTargetView* m_view = nullptr;

		class Window* m_window = nullptr;
		Size2D m_size          = {0.f, 0.f};
		bool m_with_vsync      = false;

		bool is_window_viewport() const override;
		Size2D render_target_size() const override;
		ID3D11RenderTargetView* render_target() const override;

		void init(class Window* window, bool vsync);
		void create_swapchain(const Size2D& size);

		void present() override;
		void vsync(bool flag) override;
		void on_resize(const Size2D& new_size) override;

		~D3D11_WindowViewport();
	};
}// namespace Engine
