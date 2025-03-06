#pragma once
#include <Graphics/texture_2D.hpp>


namespace Engine
{
	class ENGINE_EXPORT RenderSurface : public Texture2D
	{
		trinex_declare_class(RenderSurface, Texture2D);

	public:
		RenderSurface();

		RenderSurface& rhi_init() override;
		RenderSurface& rhi_clear_color(const Color& color);
		RenderSurface& rhi_clear_depth_stencil(float depth, byte stencil);
		RenderSurface& rhi_blit(RenderSurface* surface, const Rect2D& src, const Rect2D& dst,
								SamplerFilter filter = SamplerFilter::Trilinear);
	};
}// namespace Engine
