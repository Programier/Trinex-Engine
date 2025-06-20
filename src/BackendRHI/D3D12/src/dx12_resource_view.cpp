#include <dx12_api.hpp>
#include <dx12_resource_view.hpp>

namespace Engine
{
	D3D12_SRV::D3D12_SRV(ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc)
	{
		auto api     = D3D12::api();
		m_descriptor = api->descriptor_manager()->allocate_resource();
		api->device()->CreateShaderResourceView(resource, &desc, m_descriptor.cpu_handle());
	}

	D3D12_SRV::~D3D12_SRV()
	{
		m_descriptor.release();
	}

	D3D12_UAV::D3D12_UAV(ID3D12Resource* resource, ID3D12Resource* counter, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc)
	{
		auto api     = D3D12::api();
		m_descriptor = api->descriptor_manager()->allocate_resource();
		api->device()->CreateUnorderedAccessView(resource, counter, &desc, m_descriptor.cpu_handle());
	}

	D3D12_UAV::~D3D12_UAV()
	{
		m_descriptor.release();
	}

	D3D12_RTV::D3D12_RTV(ID3D12Resource* resource, DXGI_FORMAT format) : m_format(format)
	{
		auto api     = D3D12::api();
		m_descriptor = api->descriptor_manager()->allocate_rtv();
		api->device()->CreateRenderTargetView(resource, nullptr, m_descriptor.cpu_handle());
	}

	void D3D12_RTV::clear(const LinearColor& color)
	{
		auto cmd_list = D3D12::api()->cmd_list();
		auto handle   = descriptor().cpu_handle();
		cmd_list->ClearRenderTargetView(handle, &color.x, 0, nullptr);
	}

	void D3D12_RTV::blit(RHI_RenderTargetView* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) {}

	D3D12_RTV::~D3D12_RTV()
	{
		m_descriptor.release();
	}

	D3D12_DSV::D3D12_DSV(ID3D12Resource* resource, DXGI_FORMAT format)
	{
		auto api     = D3D12::api();
		m_descriptor = api->descriptor_manager()->allocate_dsv();

		D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
		desc.Format                        = format;
		desc.ViewDimension                 = D3D12_DSV_DIMENSION_TEXTURE2D;

		api->device()->CreateDepthStencilView(resource, &desc, m_descriptor.cpu_handle());
	}
	
	void D3D12_DSV::clear(float depth, byte stencil)
	{
		
	}
	
	void D3D12_DSV::blit(RHI_DepthStencilView* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter)
	{
		
	}

	D3D12_DSV::~D3D12_DSV()
	{
		m_descriptor.release();
	}
}// namespace Engine
