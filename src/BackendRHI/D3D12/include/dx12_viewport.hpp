#pragma once
#include <Graphics/rhi.hpp>
#include <dx12_destroyable.hpp>
#include <dx12_headers.hpp>
#include <dx12_resource_view.hpp>
#include <dxgi1_4.h>

namespace Engine
{
	class Window;
	class D3D12_Viewport : public D3D12_DeferredDestroyable<RHIViewport>
	{
		static constexpr uint_t s_frame_count = 3;

	private:
		Window* m_window             = nullptr;
		IDXGISwapChain3* m_swapchain = nullptr;
		ComPtr<ID3D12Resource> m_render_targets[s_frame_count];
		D3D12_RTV* m_rtv[s_frame_count] = {};
		D3D12_RESOURCE_STATES m_state;
		int8_t m_frame = -1;
		bool m_vsync;

	private:
		D3D12_Viewport& create_swapchain();
		D3D12_Viewport& release_swapchain();
		D3D12_Viewport& acquire_frame();

	public:
		D3D12_Viewport(WindowRenderViewport* viewport, bool vsync);
		~D3D12_Viewport();
		void present() override;

		void vsync(bool flag) override;
		void on_resize(const Size2D& new_size) override;
		void on_orientation_changed(Orientation orientation) override;
		void bind() override;
		void blit_target(RHIRenderTargetView* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
		                 SamplerFilter filter) override;
		void clear_color(const LinearColor& color) override;

		D3D12_Viewport& transition(D3D12_RESOURCE_STATES state);
		inline ID3D12Resource* resource() { return acquire_frame().m_render_targets[m_frame].Get(); }
		inline D3D12_RTV* rtv() { return acquire_frame().m_rtv[m_frame]; }
	};
}// namespace Engine
