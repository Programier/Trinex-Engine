#pragma once
#include <Graphics/rhi.hpp>
#include <dx12_destroyable.hpp>
#include <dx12_headers.hpp>

namespace Engine
{
	class D3D12_RTV;
	class D3D12_DSV;
	class D3D12_SRV;
	class D3D12_UAV;

	class D3D12Texture : public D3D12_DeferredDestroyable<RHITexture>
	{
		ComPtr<ID3D12Resource> m_resource;
		D3D12_RESOURCE_STATES m_state;
		
		D3D12_RTV* m_rtv = nullptr;
		D3D12_DSV* m_dsv = nullptr;
		D3D12_SRV* m_srv = nullptr;
		D3D12_UAV* m_uav = nullptr;

	public:
		~D3D12Texture();
		D3D12Texture& create_2d(ColorFormat format, Vector2u size, uint32_t mips, TextureCreateFlags flags);
		D3D12Texture& transition(D3D12_RESOURCE_STATES state);
		
		
		RHIRenderTargetView* as_rtv() override;
		RHIDepthStencilView* as_dsv() override;
		RHIShaderResourceView* as_srv() override;
		RHIUnorderedAccessView* as_uav() override;
	};
}// namespace Engine
