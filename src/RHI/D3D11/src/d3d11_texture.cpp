#include <Core/logger.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/texture_2D.hpp>
#include <d3d11.h>
#include <d3d11_api.hpp>
#include <d3d11_enums.hpp>
#include <d3d11_pipeline.hpp>
#include <d3d11_sampler.hpp>
#include <d3d11_texture.hpp>

namespace Engine
{
	void D3D11_Texture::clear_color(const Color& color)
	{}

	void D3D11_Texture::clear_depth_stencil(float depth, byte stencil)
	{}

	static UINT format_block_size(DXGI_FORMAT format)
	{
		switch (format)
		{
			case DXGI_FORMAT_R32_FLOAT:
				return sizeof(float);

			case DXGI_FORMAT_R32G32B32A32_FLOAT:
				return sizeof(float) * 4;

			case DXGI_FORMAT_R8_UNORM:
				return sizeof(uint8_t);

			case DXGI_FORMAT_R8G8B8A8_UNORM:
				return sizeof(uint8_t) * 4;

			case DXGI_FORMAT_D24_UNORM_S8_UINT:
				return 4;

			case DXGI_FORMAT_D32_FLOAT:
				return sizeof(float);

			case DXGI_FORMAT_R32_TYPELESS:
				return sizeof(float);

			case DXGI_FORMAT_BC1_UNORM:
				return 2;

			case DXGI_FORMAT_BC2_UNORM:
				return 4;

			case DXGI_FORMAT_BC3_UNORM:
				return 4;

			default:
				throw EngineException("Undefined format");
		}
	}

	static FORCE_INLINE Buffer flip_texture_buffer(const Buffer& buffer, int_t height, int_t pitch)
	{
		Buffer result(buffer.size());
		byte* write_to = result.data();

		for (int_t i = height - 1; i >= 0; --i)
		{
			const byte* current_row = buffer.data() + pitch * i;
			std::copy(current_row, current_row + pitch, write_to);
			write_to += pitch;
		}

		return result;
	}

	bool D3D11_Texture2D::init(const Texture2D* texture, bool is_render_surface)
	{
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width              = static_cast<UINT>(texture->width());
		desc.Height             = static_cast<UINT>(texture->height());
		desc.MipLevels          = static_cast<UINT>(texture->mipmap_count());
		desc.ArraySize          = 1;
		desc.Format             = format_of(texture->format());
		desc.SampleDesc.Count   = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage              = D3D11_USAGE_DEFAULT;
		desc.BindFlags          = desc.Format != DXGI_FORMAT_D24_UNORM_S8_UINT ? D3D11_BIND_SHADER_RESOURCE : 0;
		desc.CPUAccessFlags     = 0;
		desc.MiscFlags          = 0;

		if (is_render_surface)
		{
			if (is_in<ColorFormat::DepthStencil, ColorFormat::D32F, ColorFormat::ShadowDepth, ColorFormat::FilteredShadowDepth>(
			            texture->format()))
			{
				desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
			}
			else
			{
				desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
			}
		}

		Vector<D3D11_SUBRESOURCE_DATA> sub_resource_data;
		sub_resource_data.resize(texture->mipmap_count());

		for (size_t i = 0, count = texture->mipmap_count(); i < count; ++i)
		{
			D3D11_SUBRESOURCE_DATA& data = sub_resource_data[i];
			const auto& mip              = texture->mip(i);
			int_t pitch                  = static_cast<int_t>(mip->data.size()) / static_cast<int_t>(mip->size.y);

			Buffer buffer = flip_texture_buffer(mip->data, static_cast<int_t>(mip->size.y), pitch);
			data.pSysMem  = buffer.data();

			if (data.pSysMem == nullptr)
			{
				sub_resource_data.erase(sub_resource_data.begin() + i, sub_resource_data.end());
				break;
			}

			data.SysMemSlicePitch = 0;
			data.SysMemPitch      = static_cast<UINT>(mip->size.x) * format_block_size(desc.Format);
		}

		HRESULT hr = DXAPI->m_device->CreateTexture2D(&desc, sub_resource_data.empty() ? nullptr : sub_resource_data.data(),
		                                              &m_texture);

		if (hr != S_OK)
		{
			error_log("D3D11", "Failed to create texture!");
			return false;
		}


		if (desc.Format != DXGI_FORMAT_D24_UNORM_S8_UINT)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC view_desc{};
			view_desc.Format                    = desc.Format;
			view_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
			view_desc.Texture2D.MostDetailedMip = 0;
			view_desc.Texture2D.MipLevels       = desc.MipLevels;

			hr = DXAPI->m_device->CreateShaderResourceView(m_texture, &view_desc, &m_view);
			if (hr != S_OK)
			{
				error_log("D3D11", "Failed to texture view!");
				return false;
			}
		}

		return true;
	}

	void D3D11_Texture2D::bind(BindLocation location)
	{
		auto& pipeline = DXAPI->m_state.pipeline;

		if (pipeline == nullptr)
			return;

		if (pipeline->m_vertex_shader)
			DXAPI->m_context->VSSetShaderResources(location.binding, 1, &m_view);
		if (pipeline->m_tsc_shader)
			DXAPI->m_context->HSSetShaderResources(location.binding, 1, &m_view);
		if (pipeline->m_ts_shader)
			DXAPI->m_context->DSSetShaderResources(location.binding, 1, &m_view);
		if (pipeline->m_geometry_shader)
			DXAPI->m_context->GSSetShaderResources(location.binding, 1, &m_view);
		if (pipeline->m_fragment_shader)
			DXAPI->m_context->PSSetShaderResources(location.binding, 1, &m_view);
	}

	void D3D11_Texture2D::bind_combined(RHI_Sampler* sampler, BindLocation location)
	{
		bind(location);
		reinterpret_cast<D3D11_Sampler*>(sampler)->bind(location);
	}

	D3D11_Texture2D::~D3D11_Texture2D()
	{
		d3d11_release(m_view);
		d3d11_release(m_texture);
	}

	bool D3D11_RenderSurface::init(const RenderSurface* surface)
	{
		if (!D3D11_Texture2D::init(surface, true))
		{
			return false;
		}

		if (is_in<ColorFormat::DepthStencil, ColorFormat::D32F, ColorFormat::ShadowDepth, ColorFormat::FilteredShadowDepth>(
		            surface->format()))
		{
			m_depth_stencil_view = DXAPI->create_depth_stencil_view(m_texture, format_of(surface->format()));

			if (m_depth_stencil_view == nullptr)
			{
				error_log("D3D11 Render Surface", "Failed ot create depth stencil view");
				return false;
			}
		}
		else
		{
			m_render_target = DXAPI->create_render_target_view(m_texture, format_of(surface->format()));

			if (m_render_target == nullptr)
			{
				error_log("D3D11 Render Surface", "Failed ot create render target view");
				return false;
			}
		}

		return true;
	}

	void D3D11_RenderSurface::clear_color(const Color& color)
	{
		if (m_render_target)
		{
			DXAPI->m_context->ClearRenderTargetView(m_render_target, &color.x);
		}
	}

	void D3D11_RenderSurface::clear_depth_stencil(float depth, byte stencil)
	{
		if (m_depth_stencil_view)
		{
			DXAPI->m_context->ClearDepthStencilView(m_depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth,
			                                        stencil);
		}
	}

	D3D11_RenderSurface::~D3D11_RenderSurface()
	{
		d3d11_release(m_render_target);
		d3d11_release(m_depth_stencil_view);
	}

	RHI_Texture* D3D11::create_texture_2d(const Texture2D* texture)
	{
		D3D11_Texture2D* d3d11_texture = new D3D11_Texture2D();

		if (!d3d11_texture->init(texture))
		{
			delete d3d11_texture;
			d3d11_texture = nullptr;
		}

		return d3d11_texture;
	}

	RHI_Texture* D3D11::create_render_surface(const RenderSurface* surface)
	{
		D3D11_RenderSurface* d3d11_surface = new D3D11_RenderSurface();

		if (!d3d11_surface->init(surface))
		{
			delete d3d11_surface;
			d3d11_surface = nullptr;
		}

		return d3d11_surface;
	}

	D3D11& D3D11::bind_render_target(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil)
	{
		static ID3D11RenderTargetView* render_target_views[RHI_MAX_RT_BINDED];

		size_t size = 0;

		for (auto& surface : color_attachments)
		{
			render_target_views[size] = surface->rhi_object<D3D11_RenderSurface>()->m_render_target;
			++size;
		}

		D3D11_Pipeline::unbind();

		m_context->OMSetRenderTargets(size, render_target_views,
		                              depth_stencil ? depth_stencil->rhi_object<D3D11_RenderSurface>()->m_depth_stencil_view
		                                            : nullptr);

		if (current_viewport_mode() != m_state.viewport_mode)
		{
			viewport(m_state.viewport);
		}
		return *this;
	}

}// namespace Engine
