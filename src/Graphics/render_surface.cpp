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
			TextureCreateFlags flags = TextureCreateFlags::ShaderResource;

			if (m_format.is_color())
			{
				flags |= TextureCreateFlags::RenderTarget;
				flags |= TextureCreateFlags::UnorderedAccess;
			}
			else if (m_format.is_depth())
			{
				flags |= TextureCreateFlags::DepthStencilTarget;
			}

			m_texture = rhi->create_texture_2d(m_format, m_size, 1, flags);

			if (flags & TextureCreateFlags::ShaderResource)
				m_srv = m_texture->create_srv();
			if (flags & TextureCreateFlags::UnorderedAccess)
				m_uav = m_texture->create_uav();
			if (flags & TextureCreateFlags::RenderTarget)
				m_rtv = m_texture->create_rtv();
			if (flags & TextureCreateFlags::DepthStencilTarget)
				m_dsv = m_texture->create_dsv();
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
		m_texture = nullptr;

		m_format = ColorFormat::Undefined;
		m_size   = {0, 0};
		return *this;
	}
}// namespace Engine
