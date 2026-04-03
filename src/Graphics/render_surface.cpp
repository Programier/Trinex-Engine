#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Graphics/render_surface.hpp>
#include <RHI/initializers.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
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

		RHITextureFlags flags = RHITextureFlags::ShaderResource;

		if (m_format.is_color())
		{
			flags |= RHITextureFlags::RenderTarget;
			flags |= RHITextureFlags::UnorderedAccess;
		}
		else if (m_format.has_depth())
		{
			flags |= RHITextureFlags::DepthStencilTarget;
		}

		RHITextureDesc desc = {
		        .type   = RHITextureType::Texture2D,
		        .format = RHIColorFormat(m_format),
		        .size   = {m_size, 1},
		        .mips   = 1,
		        .flags  = flags,
		};

		m_texture = RHI::instance()->create_texture(desc);
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
}// namespace Trinex
