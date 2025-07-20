#pragma once
#include <Graphics/rhi.hpp>
#include <dx12_descriptor.hpp>

namespace Engine
{
	class D3D12_SRV : public RHIShaderResourceView
	{
	private:
		D3D12Descriptor m_descriptor;

	public:
		D3D12_SRV(ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
		~D3D12_SRV();
		inline const D3D12Descriptor& descriptor() const { return m_descriptor; }
	};

	class D3D12_UAV : public RHIUnorderedAccessView
	{
	private:
		D3D12Descriptor m_descriptor;

	public:
		D3D12_UAV(ID3D12Resource* resource, ID3D12Resource* counter, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc);
		~D3D12_UAV();

		inline const D3D12Descriptor& descriptor() const { return m_descriptor; }
	};

	class D3D12_RTV : public RHIRenderTargetView
	{
	private:
		D3D12Descriptor m_descriptor;
		DXGI_FORMAT m_format;

	public:
		D3D12_RTV(ID3D12Resource* resource, DXGI_FORMAT format);
		~D3D12_RTV();

		void clear(const LinearColor& color) override;
		void blit(RHIRenderTargetView* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override;

		inline const D3D12Descriptor& descriptor() const { return m_descriptor; }
		inline DXGI_FORMAT format() const { return m_format; }
	};

	class D3D12_DSV : public RHIDepthStencilView
	{
	private:
		D3D12Descriptor m_descriptor;
		DXGI_FORMAT m_format;

	public:
		D3D12_DSV(ID3D12Resource* resource, DXGI_FORMAT format);
		~D3D12_DSV();

		void clear(float depth, byte stencil) override;
		void blit(RHIDepthStencilView* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override;

		inline const D3D12Descriptor& descriptor() const { return m_descriptor; }
		inline DXGI_FORMAT format() const { return m_format; }
	};
}// namespace Engine
