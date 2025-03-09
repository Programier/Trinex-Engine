#pragma once
#include <Graphics/rhi.hpp>
#include <d3d11.h>

namespace Engine
{
	class D3D11_Surface : public RHI_DefaultDestroyable<RHI_Surface>
	{
	public:
		Vector2u m_size;
		ID3D11Texture2D* m_texture = nullptr;

		bool init(ColorFormat format, Vector2u size);
		RHI_RenderTargetView* create_rtv() override;
		RHI_DepthStencilView* create_dsv() override;
		RHI_ShaderResourceView* create_srv() override;
		RHI_UnorderedAccessView* create_uav() override;
		~D3D11_Surface();
	};

	class D3d11_SurfaceRTV : public RHI_DefaultDestroyable<RHI_RenderTargetView>
	{
	public:
		D3D11_Surface* m_surface;
		ID3D11RenderTargetView* m_view;

		D3d11_SurfaceRTV(D3D11_Surface* surface, ID3D11RenderTargetView* view);
		void clear(const Color& color) override;
		void blit(RHI_RenderTargetView* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override;
		~D3d11_SurfaceRTV();
	};

	class D3d11_SurfaceDSV : public RHI_DefaultDestroyable<RHI_DepthStencilView>
	{
	public:
		D3D11_Surface* m_surface;
		ID3D11DepthStencilView* m_view;

		D3d11_SurfaceDSV(D3D11_Surface* surface, ID3D11DepthStencilView* view);
		void clear(float depth, byte stencil) override;
		void blit(RHI_DepthStencilView* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override;
		~D3d11_SurfaceDSV();
	};
}// namespace Engine
