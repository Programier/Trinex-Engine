#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/rhi.hpp>


namespace Engine
{
	implement_engine_class_default_init(RenderSurface, 0);

	RenderSurface::RenderSurface()
	{
		flags(IsSerializable, false);
		flags(IsEditable, false);
	}

	RenderSurface& RenderSurface::rhi_create()
	{
		m_rhi_object.reset(rhi->create_render_surface(this));
		return *this;
	}

	RenderSurface& RenderSurface::rhi_clear_color(const Color& color)
	{
		if (has_object() &&
			!is_in<ColorFormat::D32F, ColorFormat::DepthStencil, ColorFormat::ShadowDepth, ColorFormat::FilteredShadowDepth>(
					format()))
		{
			rhi_object<RHI_Texture>()->clear_color(color);
		}
		return *this;
	}

	RenderSurface& RenderSurface::rhi_clear_depth_stencil(float depth, byte stencil)
	{
		if (has_object() &&
			is_in<ColorFormat::D32F, ColorFormat::DepthStencil, ColorFormat::ShadowDepth, ColorFormat::FilteredShadowDepth>(
					format()))
		{
			rhi_object<RHI_Texture>()->clear_depth_stencil(depth, stencil);
		}
		return *this;
	}

}// namespace Engine
