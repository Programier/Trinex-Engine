#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
	trinex_implement_engine_class_default_init(RenderSurface, 0);

	RenderSurface::RenderSurface()
	{
		flags(IsSerializable, false);
		flags(IsEditable, false);
	}

	RenderSurface& RenderSurface::init(ColorFormat format, Vector2i size)
	{
		m_format = format;
		m_size   = size;

		render_thread()->call([this]() {
			m_surface = rhi->create_render_surface(m_format, m_size);
			if (m_format.is_color())
				m_rtv = m_surface->create_rtv();
			else if (m_format.is_depth())
				m_dsv = m_surface->create_dsv();
		});

		return *this;
	}

	RenderSurface& RenderSurface::release_render_resources()
	{
		Super::release_render_resources();
		m_uav     = nullptr;
		m_srv     = nullptr;
		m_dsv     = nullptr;
		m_rtv     = nullptr;
		m_surface = nullptr;

		m_format = ColorFormat::Undefined;
		m_size   = {0, 0};
		return *this;
	}

	RHI_ShaderResourceView* RenderSurface::rhi_shader_resource_view() const
	{
		if (!m_srv && m_surface)
		{
			m_srv = m_surface->create_srv();
		}
		return m_srv;
	}

	RHI_UnorderedAccessView* RenderSurface::rhi_unordered_access_view() const
	{
		if (!m_uav && m_surface)
		{
			m_uav = m_surface->create_uav();
		}
		return m_uav;
	}
}// namespace Engine
