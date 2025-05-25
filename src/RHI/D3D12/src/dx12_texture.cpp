#include <Core/memory.hpp>
#include <dx12_api.hpp>
#include <dx12_enums.hpp>
#include <dx12_resource_view.hpp>
#include <dx12_texture.hpp>

namespace Engine
{
	D3D12Texture::~D3D12Texture()
	{
		if (m_rtv)
			Engine::release(m_rtv);
		if (m_dsv)
			Engine::release(m_dsv);
		if (m_srv)
			Engine::release(m_srv);
		if (m_uav)
			Engine::release(m_uav);
	}

	D3D12Texture& D3D12Texture::create_2d(ColorFormat format, Vector2u size, uint32_t mips, TextureCreateFlags flags)
	{
		D3D12_RESOURCE_DESC desc = {};
		desc.MipLevels           = mips;
		desc.Format              = texture_format_of(format);
		desc.Width               = size.x;
		desc.Height              = size.y;
		desc.Flags               = D3D12_RESOURCE_FLAG_NONE;
		desc.DepthOrArraySize    = 1;
		desc.SampleDesc.Count    = 1;
		desc.SampleDesc.Quality  = 0;
		desc.Dimension           = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		if (flags & TextureCreateFlags::RenderTarget)
			desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		if (flags & TextureCreateFlags::DepthStencilTarget)
			desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		if (flags & TextureCreateFlags::UnorderedAccess)
			desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		D3D12_HEAP_PROPERTIES heap_properties;
		heap_properties.Type                 = D3D12_HEAP_TYPE_DEFAULT;
		heap_properties.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_properties.CreationNodeMask     = 1;
		heap_properties.VisibleNodeMask      = 1;

		m_state = D3D12_RESOURCE_STATE_COPY_DEST;
		D3D12::api()->device()->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &desc, m_state, nullptr,
		                                                IID_PPV_ARGS(&m_resource));

		if (flags & TextureCreateFlags::ShaderResource)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
			srv_desc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srv_desc.Format                          = graphics_view_format_of(desc.Format);
			srv_desc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
			srv_desc.Texture2D.MostDetailedMip       = 0;
			srv_desc.Texture2D.MipLevels             = mips;
			srv_desc.Texture2D.ResourceMinLODClamp   = 0.0f;

			m_srv = allocate<D3D12_SRV>(m_resource.Get(), srv_desc);
		}

		if (flags & TextureCreateFlags::UnorderedAccess)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
			uav_desc.Format                           = graphics_view_format_of(desc.Format);
			uav_desc.ViewDimension                    = D3D12_UAV_DIMENSION_TEXTURE2D;
			uav_desc.Texture2D.MipSlice               = 0;
			uav_desc.Texture2D.PlaneSlice             = 0;

			m_uav = allocate<D3D12_UAV>(m_resource.Get(), nullptr, uav_desc);
		}

		if (flags & TextureCreateFlags::RenderTarget)
		{
			m_rtv = allocate<D3D12_RTV>(m_resource.Get(), render_view_format_of(desc.Format));
		}

		if (flags & TextureCreateFlags::DepthStencilTarget)
		{
			m_dsv = allocate<D3D12_DSV>(m_resource.Get(), render_view_format_of(desc.Format));
		}

		return *this;
	}

	D3D12Texture& D3D12Texture::transition(D3D12_RESOURCE_STATES state)
	{
		if (m_state != state)
		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource   = m_resource.Get();
			barrier.Transition.StateBefore = m_state;
			barrier.Transition.StateAfter  = state;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			D3D12::api()->cmd_list()->ResourceBarrier(1, &barrier);

			m_state = state;
		}

		return *this;
	}

	RHI_RenderTargetView* D3D12Texture::as_rtv()
	{
		return m_rtv;
	}

	RHI_DepthStencilView* D3D12Texture::as_dsv()
	{
		return m_dsv;
	}

	RHI_ShaderResourceView* D3D12Texture::as_srv()
	{
		return m_srv;
	}

	RHI_UnorderedAccessView* D3D12Texture::as_uav()
	{
		return m_uav;
	}

	D3D12& D3D12::barrier(RHI_Texture* texture, RHIAccess access)
	{
		static_cast<D3D12Texture*>(texture)->transition(resource_state_of(access));
		return *this;
	}

	RHI_Texture* D3D12::create_texture_2d(ColorFormat format, Vector2u size, uint32_t mips, TextureCreateFlags flags)
	{
		return &allocate<D3D12Texture>()->create_2d(format, size, mips, flags);
	}
}// namespace Engine
