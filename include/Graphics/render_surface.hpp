#pragma once
#include <Core/render_resource.hpp>

namespace Engine
{
	struct RHI_Surface;
	struct RHI_ShaderResourceView;
	struct RHI_UnorderedAccessView;
	struct RHI_RenderTargetView;
	struct RHI_DepthStencilView;

	class ENGINE_EXPORT RenderSurface : public RenderResource
	{
		trinex_declare_class(RenderSurface, RenderResource);

		RenderResourcePtr<RHI_Surface> m_surface;
		RenderResourcePtr<RHI_RenderTargetView> m_rtv;
		RenderResourcePtr<RHI_DepthStencilView> m_dsv;
		mutable RenderResourcePtr<RHI_ShaderResourceView> m_srv;
		mutable RenderResourcePtr<RHI_UnorderedAccessView> m_uav;

		ColorFormat m_format = ColorFormat::Undefined;
		Vector2u m_size      = {0, 0};

	public:
		RenderSurface();
		RenderSurface& init(ColorFormat format, Vector2i size);
		RenderSurface& release_render_resources() override;

		RHI_ShaderResourceView* rhi_shader_resource_view() const;
		RHI_UnorderedAccessView* rhi_unordered_access_view() const;

		inline RHI_Surface* rhi_surface() const { return m_surface; }
		inline RHI_RenderTargetView* rhi_render_target_view() const { return m_rtv; }
		inline RHI_DepthStencilView* rhi_depth_stencil_view() const { return m_dsv; }
		inline Vector2u size() const { return m_size; }
		inline ColorFormat format() const { return m_format; }
	};
}// namespace Engine
