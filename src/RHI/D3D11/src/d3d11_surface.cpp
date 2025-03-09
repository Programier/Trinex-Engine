#include <Core/logger.hpp>
#include <d3d11_api.hpp>
#include <d3d11_enums.hpp>
#include <d3d11_surface.hpp>
#include <d3d11_texture.hpp>

namespace Engine
{
	bool D3D11_Surface::init(ColorFormat format, Vector2u size)
	{
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width              = size.x;
		desc.Height             = size.y;
		desc.MipLevels          = 1;
		desc.ArraySize          = 1;
		desc.Format             = texture_format_of(format);
		desc.SampleDesc.Count   = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage              = D3D11_USAGE_DEFAULT;
		desc.BindFlags          = desc.Format != DXGI_FORMAT_D24_UNORM_S8_UINT ? D3D11_BIND_SHADER_RESOURCE : 0;
		desc.CPUAccessFlags     = 0;
		desc.MiscFlags          = 0;

		if (format.is_depth())
		{
			desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
		}
		else
		{
			desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		}

		HRESULT hr = DXAPI->m_device->CreateTexture2D(&desc, nullptr, &m_texture);

		if (hr != S_OK)
		{
			error_log("D3D11", "Failed to create surface!");
			return false;
		}

		m_size = size;
		return true;
	}

	RHI_RenderTargetView* D3D11_Surface::create_rtv()
	{
		D3D11_TEXTURE2D_DESC desc;
		m_texture->GetDesc(&desc);

		auto view = DXAPI->create_render_target_view(m_texture, render_view_format_of(desc.Format));

		if (view == nullptr)
		{
			error_log("D3D11 Render Surface", "Failed ot create depth stencil view");
			return nullptr;
		}

		return new D3d11_SurfaceRTV(this, view);
	}

	RHI_DepthStencilView* D3D11_Surface::create_dsv()
	{
		D3D11_TEXTURE2D_DESC desc;
		m_texture->GetDesc(&desc);

		auto view = DXAPI->create_depth_stencil_view(m_texture, render_view_format_of(desc.Format));

		if (view == nullptr)
		{
			error_log("D3D11 Render Surface", "Failed ot create depth stencil view");
			return nullptr;
		}

		return new D3d11_SurfaceDSV(this, view);
	}

	RHI_ShaderResourceView* D3D11_Surface::create_srv()
	{
		return D3D11_Texture2D::static_create_srv(this, m_texture);
	}

	RHI_UnorderedAccessView* D3D11_Surface::create_uav()
	{
		return D3D11_Texture2D::static_create_uav(this, m_texture);
	}

	D3D11_Surface::~D3D11_Surface()
	{
		m_texture->Release();
	}

	D3d11_SurfaceRTV::D3d11_SurfaceRTV(D3D11_Surface* surface, ID3D11RenderTargetView* view) : m_surface(surface), m_view(view)
	{
		m_surface->add_reference();
	}

	void D3d11_SurfaceRTV::clear(const Color& color)
	{
		DXAPI->m_context->ClearRenderTargetView(m_view, &color.r);
	}

	void D3d11_SurfaceRTV::blit(RHI_RenderTargetView* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
								SamplerFilter filter)
	{}

	D3d11_SurfaceRTV::~D3d11_SurfaceRTV()
	{
		m_surface->release();
	}

	D3d11_SurfaceDSV::D3d11_SurfaceDSV(D3D11_Surface* surface, ID3D11DepthStencilView* view) : m_surface(surface), m_view(view)
	{
		m_surface->add_reference();
	}

	void D3d11_SurfaceDSV::clear(float depth, byte stencil)
	{
		DXAPI->m_context->ClearDepthStencilView(m_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
	}

	void D3d11_SurfaceDSV::blit(RHI_DepthStencilView* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
								SamplerFilter filter)
	{}

	D3d11_SurfaceDSV::~D3d11_SurfaceDSV()
	{
		m_surface->release();
	}

	RHI_Surface* D3D11::create_render_surface(ColorFormat format, Vector2u size)
	{
		D3D11_Surface* d3d11_surface = new D3D11_Surface();

		if (!d3d11_surface->init(format, size))
		{
			delete d3d11_surface;
			d3d11_surface = nullptr;
		}

		return d3d11_surface;
	}

	static FORCE_INLINE ID3D11RenderTargetView* view_of(RHI_RenderTargetView* rt)
	{
		return rt ? static_cast<D3d11_SurfaceRTV*>(rt)->m_view : nullptr;
	}

	static FORCE_INLINE ID3D11DepthStencilView* view_of(RHI_DepthStencilView* dsv)
	{
		return dsv ? static_cast<D3d11_SurfaceDSV*>(dsv)->m_view : nullptr;
	}

	static FORCE_INLINE D3D11_Surface* surface_of(RHI_RenderTargetView* rt)
	{
		return rt ? static_cast<D3d11_SurfaceRTV*>(rt)->m_surface : nullptr;
	}

	static FORCE_INLINE D3D11_Surface* surface_of(RHI_DepthStencilView* dsv)
	{
		return dsv ? static_cast<D3d11_SurfaceDSV*>(dsv)->m_surface : nullptr;
	}

	static FORCE_INLINE Vector2i static_find_render_target_size(RHI_RenderTargetView* rt1, RHI_RenderTargetView* rt2,
																RHI_RenderTargetView* rt3, RHI_RenderTargetView* rt4,
																RHI_DepthStencilView* depth_stencil)
	{
		if (auto surface = surface_of(rt1))
			return surface->m_size;
		if (auto surface = surface_of(rt2))
			return surface->m_size;
		if (auto surface = surface_of(rt3))
			return surface->m_size;
		if (auto surface = surface_of(rt4))
			return surface->m_size;
		if (auto surface = surface_of(depth_stencil))
			return surface->m_size;
		return {-1, -1};
	}

	D3D11& D3D11::bind_render_target(RHI_RenderTargetView* rt1, RHI_RenderTargetView* rt2, RHI_RenderTargetView* rt3,
									 RHI_RenderTargetView* rt4, RHI_DepthStencilView* depth_stencil)
	{
		static ID3D11RenderTargetView* render_target_views[4] = {view_of(rt1), view_of(rt2), view_of(rt3), view_of(rt4)};

		m_context->OMSetRenderTargets(4, render_target_views, view_of(depth_stencil));
		m_state.render_target_size = static_find_render_target_size(rt1, rt2, rt3, rt4, depth_stencil);
		viewport(m_state.viewport);
		scissor(m_state.scissor);
		return *this;
	}
}// namespace Engine
