#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Graphics/render_surface.hpp>
#include <RHI/enums.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
	trinex_implement_engine_class_default_init(RenderSurface, 0);

	RenderSurface::RenderSurface()
	{
		flags(IsSerializable, false);
		flags(IsEditable, false);
	}

	RenderSurface& RenderSurface::init(RHISurfaceFormat format, Vector2i size)
	{
		m_format = format;
		m_size   = size;

		RHITextureCreateFlags flags = RHITextureCreateFlags::ShaderResource;

		if (m_format.is_color())
		{
			flags |= RHITextureCreateFlags::RenderTarget;
			flags |= RHITextureCreateFlags::UnorderedAccess;
		}
		else if (m_format.has_depth())
		{
			flags |= RHITextureCreateFlags::DepthStencilTarget;
		}

		m_texture = rhi->create_texture(RHITextureType::Texture2D, RHIColorFormat(m_format), {m_size, 1}, 1, flags);
		return *this;
	}

	RenderSurface& RenderSurface::release_render_resources()
	{
		Super::release_render_resources();
		m_texture = nullptr;

		m_format = RHISurfaceFormat::Undefined;
		m_size   = {0, 0};
		return *this;
	}

	RHIRenderTargetView* RenderSurface::rhi_rtv() const
	{
		return m_texture ? m_texture->as_rtv() : nullptr;
	}

	RHIDepthStencilView* RenderSurface::rhi_dsv() const
	{
		return m_texture ? m_texture->as_dsv() : nullptr;
	}

	RHIUnorderedAccessView* RenderSurface::rhi_uav() const
	{
		return m_texture ? m_texture->as_uav() : nullptr;
	}

	RHIShaderResourceView* RenderSurface::rhi_srv() const
	{
		return m_texture ? m_texture->as_srv() : nullptr;
	}
}// namespace Engine
