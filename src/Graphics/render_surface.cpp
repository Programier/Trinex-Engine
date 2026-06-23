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
		flags.remove(Flags::IsSerializable | Flags::IsEditable);
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

	RenderSurface& RenderSurface::rebuild()
	{
		if (m_format == RHISurfaceFormat::Undefined || m_size == Vector2u(0, 0))
			return *this;

		return init(m_format, Vector2i(m_size));
	}

	RHIRenderTargetView* RenderSurface::rtv() const
	{
		return m_texture ? m_texture->as_rtv() : nullptr;
	}

	RHIDepthStencilView* RenderSurface::dsv() const
	{
		return m_texture ? m_texture->as_dsv() : nullptr;
	}

	RHIUnorderedAccessView* RenderSurface::uav() const
	{
		return m_texture ? m_texture->as_uav() : nullptr;
	}

	RHIShaderResourceView* RenderSurface::srv() const
	{
		return m_texture ? m_texture->as_srv() : nullptr;
	}
}// namespace Trinex
