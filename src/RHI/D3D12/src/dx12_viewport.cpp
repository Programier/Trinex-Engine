#include <Core/exception.hpp>
#include <Core/memory.hpp>
#include <Graphics/render_viewport.hpp>
#include <Window/window.hpp>
#include <dx12_api.hpp>
#include <dx12_viewport.hpp>

namespace Engine
{
	extern HWND extract_d3dx12_hwnd(class Window* window);

	D3D12_Viewport::D3D12_Viewport(WindowRenderViewport* viewport, bool vsync) : m_window(viewport->window()), m_vsync(vsync)
	{
		create_swapchain();
	}

	D3D12_Viewport::~D3D12_Viewport()
	{
		release_swapchain();
	}

	D3D12_Viewport& D3D12_Viewport::create_swapchain()
	{
		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
		swap_chain_desc.BufferCount           = s_frame_count;
		swap_chain_desc.Width                 = m_window->width();
		swap_chain_desc.Height                = m_window->height();
		swap_chain_desc.Format                = DXGI_FORMAT_R8G8B8A8_UNORM;
		swap_chain_desc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_desc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swap_chain_desc.SampleDesc.Count      = 1;

		HWND hwnd = extract_d3dx12_hwnd(m_window);
		IDXGISwapChain1* temp_swapchain;

		auto api = D3D12::api();

		auto result = api->factory()->CreateSwapChainForHwnd(api->command_queue(), hwnd, &swap_chain_desc, nullptr, nullptr,
		                                                     &temp_swapchain);
		trinex_always_check(result == S_OK, "Failed to create swap chain");

		result = temp_swapchain->QueryInterface(&m_swapchain);
		trinex_always_check(result == S_OK, "Failed to create swap chain");

		for (uint32_t i = 0; i < s_frame_count; i++)
		{
			if (FAILED(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_render_targets[i]))))
			{
				throw EngineException("Failed to get back buffer");
			}

			m_rtv[i] = allocate<D3D12_RTV>(m_render_targets[i].Get(), DXGI_FORMAT_R8G8B8A8_UNORM);
		}

		return *this;
	}

	D3D12_Viewport& D3D12_Viewport::release_swapchain()
	{
		m_swapchain->Release();
		m_swapchain = nullptr;

		for (auto& resource : m_render_targets)
		{
			resource.Reset();
		}

		for (D3D12_RTV*& rtv : m_rtv)
		{
			Engine::release(rtv);
			rtv = nullptr;
		}
		return *this;
	}

	D3D12_Viewport& D3D12_Viewport::acquire_frame()
	{
		if (m_frame == -1)
		{
			m_frame = m_swapchain->GetCurrentBackBufferIndex();
			m_state = D3D12_RESOURCE_STATE_PRESENT;
		}
		return *this;
	}

	void D3D12_Viewport::present()
	{
		transition(D3D12_RESOURCE_STATE_PRESENT);
		D3D12::api()->submit();
		m_swapchain->Present(m_vsync ? 1 : 0, 0);
		m_frame = -1;
	}

	void D3D12_Viewport::vsync(bool flag)
	{
		m_vsync = flag;
	}

	void D3D12_Viewport::on_resize(const Size2D& new_size)
	{
		release_swapchain().create_swapchain();
	}

	void D3D12_Viewport::on_orientation_changed(Orientation orientation) {}

	void D3D12_Viewport::bind()
	{
		transition(D3D12_RESOURCE_STATE_RENDER_TARGET);
		D3D12::api()->bind_render_target1(rtv());
	}

	void D3D12_Viewport::blit_target(RHI_RenderTargetView* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
	                                 SamplerFilter filter)
	{}

	void D3D12_Viewport::clear_color(const LinearColor& color)
	{
		transition(D3D12_RESOURCE_STATE_RENDER_TARGET);
		rtv()->clear(color);
	}

	D3D12_Viewport& D3D12_Viewport::transition(D3D12_RESOURCE_STATES state)
	{
		auto frame = resource();

		if (state != m_state)
		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource   = frame;
			barrier.Transition.StateBefore = m_state;
			barrier.Transition.StateAfter  = state;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			D3D12::api()->cmd_list()->ResourceBarrier(1, &barrier);
			m_state = state;
		}

		return *this;
	}

	RHI_Viewport* D3D12::create_viewport(WindowRenderViewport* viewport, bool vsync)
	{
		return allocate<D3D12_Viewport>(viewport, vsync);
	}
}// namespace Engine
