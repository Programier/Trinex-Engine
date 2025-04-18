#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
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

	bool D3D11_Texture2D::init(const Texture2D* texture)
	{
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width              = static_cast<UINT>(texture->width());
		desc.Height             = static_cast<UINT>(texture->height());
		desc.MipLevels          = static_cast<UINT>(texture->mips.size());
		desc.ArraySize          = 1;
		desc.Format             = texture_format_of(texture->format);
		desc.SampleDesc.Count   = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage              = D3D11_USAGE_DEFAULT;
		desc.BindFlags          = desc.Format != DXGI_FORMAT_D24_UNORM_S8_UINT ? D3D11_BIND_SHADER_RESOURCE : 0;
		desc.CPUAccessFlags     = 0;
		desc.MiscFlags          = 0;

		Vector<D3D11_SUBRESOURCE_DATA> sub_resource_data;
		Vector<Buffer> flipped_mip_data;

		sub_resource_data.resize(texture->mips.size());
		flipped_mip_data.resize(texture->mips.size());

		for (size_t i = 0, count = texture->mips.size(); i < count; ++i)
		{
			D3D11_SUBRESOURCE_DATA& data = sub_resource_data[i];
			const auto& mip              = texture->mips[i];
			int_t pitch                  = static_cast<int_t>(mip.data.size()) / static_cast<int_t>(mip.size.y);

			flipped_mip_data[i] = flip_texture_buffer(mip.data, static_cast<int_t>(mip.size.y), pitch);
			data.pSysMem        = flipped_mip_data[i].data();

			if (data.pSysMem == nullptr)
			{
				sub_resource_data.erase(sub_resource_data.begin() + i, sub_resource_data.end());
				break;
			}

			data.SysMemSlicePitch = 0;
			data.SysMemPitch      = static_cast<UINT>(mip.size.x) * format_block_size(desc.Format);
		}

		HRESULT hr = DXAPI->m_device->CreateTexture2D(&desc, sub_resource_data.empty() ? nullptr : sub_resource_data.data(),
		                                              &m_texture);

		if (hr != S_OK)
		{
			error_log("D3D11", "Failed to create texture!");
			return false;
		}

		return true;
	}

	RHI_ShaderResourceView* D3D11_Texture2D::static_create_srv(RHI_Object* owner, ID3D11Texture2D* texture)
	{
		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);

		D3D11_SHADER_RESOURCE_VIEW_DESC view_desc{};
		view_desc.Format                    = view_format_of(desc.Format);
		view_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
		view_desc.Texture2D.MostDetailedMip = 0;
		view_desc.Texture2D.MipLevels       = desc.MipLevels;

		ID3D11ShaderResourceView* view = nullptr;
		auto hr                        = DXAPI->m_device->CreateShaderResourceView(texture, &view_desc, &view);
		if (hr != S_OK)
		{
			error_log("D3D11", "Failed to SRV!");
			return nullptr;
		}

		return new D3D11_TextureSRV(owner, view);
	}

	RHI_UnorderedAccessView* D3D11_Texture2D::static_create_uav(RHI_Object* owner, ID3D11Texture2D* texture)
	{
		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);

		D3D11_UNORDERED_ACCESS_VIEW_DESC view_desc{};
		view_desc.Format             = view_format_of(desc.Format);
		view_desc.ViewDimension      = D3D11_UAV_DIMENSION_TEXTURE2D;
		view_desc.Texture2D.MipSlice = 0;

		ID3D11UnorderedAccessView* view = nullptr;
		auto hr                         = DXAPI->m_device->CreateUnorderedAccessView(texture, &view_desc, &view);
		if (hr != S_OK)
		{
			error_log("D3D11", "Failed to UAV!");
			return nullptr;
		}

		return new D3D11_TextureUAV(owner, view);
	}

	RHI_ShaderResourceView* D3D11_Texture2D::create_srv()
	{
		return static_create_srv(this, m_texture);
	}

	RHI_UnorderedAccessView* D3D11_Texture2D::create_uav()
	{
		return static_create_uav(this, m_texture);
	}

	D3D11_Texture2D::~D3D11_Texture2D()
	{
		d3d11_release(m_texture);
	}

	D3D11_TextureSRV::D3D11_TextureSRV(RHI_Object* owner, ID3D11ShaderResourceView* view) : m_owner(owner), m_view(view)
	{
		owner->add_reference();
	}

	void D3D11_TextureSRV::bind(BindLocation location)
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

	void D3D11_TextureSRV::bind_combined(byte location, struct RHI_Sampler* sampler)
	{
		bind(location);
		reinterpret_cast<D3D11_Sampler*>(sampler)->bind(location);
	}

	D3D11_TextureSRV::~D3D11_TextureSRV()
	{
		d3d11_release(m_view);
		m_owner->release();
	}

	D3D11_TextureUAV::D3D11_TextureUAV(RHI_Object* owner, ID3D11UnorderedAccessView* view) : m_owner(owner), m_view(view)
	{
		owner->add_reference();
	}

	void D3D11_TextureUAV::bind(BindLocation location) {}

	D3D11_TextureUAV::~D3D11_TextureUAV()
	{
		d3d11_release(m_view);
		m_owner->release();
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
}// namespace Engine
