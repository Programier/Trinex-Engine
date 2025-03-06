#include <Core/base_engine.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Core/etl/templates.hpp>
#include <Core/reflection/class.hpp>
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

	RenderSurface& RenderSurface::rhi_init()
	{
		m_rhi_object.reset(rhi->create_render_surface(this));
		return *this;
	}

	RenderSurface& RenderSurface::rhi_clear_color(const Color& color)
	{
		if (has_object() && !is_in<ColorFormat::Depth, ColorFormat::DepthStencil, ColorFormat::ShadowDepth>(format()))
		{
			rhi_object<RHI_Texture2D>()->clear_color(color);
		}
		return *this;
	}

	RenderSurface& RenderSurface::rhi_clear_depth_stencil(float depth, byte stencil)
	{
		if (has_object() && is_in<ColorFormat::Depth, ColorFormat::DepthStencil, ColorFormat::ShadowDepth>(format()))
		{
			rhi_object<RHI_Texture2D>()->clear_depth_stencil(depth, stencil);
		}
		return *this;
	}

	RenderSurface& RenderSurface::rhi_blit(RenderSurface* surface, const Rect2D& src, const Rect2D& dst, SamplerFilter filter)
	{
		if (auto self_surface = rhi_object<RHI_Texture2D>())
		{
			self_surface->blit(surface, src, dst, filter);
		}
		return *this;
	}
}// namespace Engine
