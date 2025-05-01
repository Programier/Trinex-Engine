#pragma once
#include <Core/render_resource.hpp>
#include <Graphics/types/color_format.hpp>

namespace Engine
{
	struct RHI_ShaderResourceView;
	struct RHI_UnorderedAccessView;
	struct RHI_RenderTargetView;
	struct RHI_DepthStencilView;
	struct RHI_Texture2D;

	class ENGINE_EXPORT RenderSurface : public RenderResource
	{
		trinex_declare_class(RenderSurface, RenderResource);

		RenderResourcePtr<RHI_Texture2D> m_texture;

		SurfaceFormat m_format = SurfaceFormat::Undefined;
		Vector2u m_size        = {0, 0};

	public:
		RenderSurface();
		RenderSurface& init(SurfaceFormat format, Vector2i size);
		RenderSurface& release_render_resources() override;
		RHI_RenderTargetView* rhi_rtv() const;
		RHI_DepthStencilView* rhi_dsv() const;
		RHI_UnorderedAccessView* rhi_uav() const;
		RHI_ShaderResourceView* rhi_srv() const;

		inline RHI_Texture2D* rhi_texture() const { return m_texture; }
		inline Vector2u size() const { return m_size; }
		inline SurfaceFormat format() const { return m_format; }
	};
}// namespace Engine
