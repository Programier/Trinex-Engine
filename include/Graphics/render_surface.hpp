#pragma once
#include <Core/render_resource.hpp>

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
		RenderResourcePtr<RHI_RenderTargetView> m_rtv;
		RenderResourcePtr<RHI_DepthStencilView> m_dsv;
		RenderResourcePtr<RHI_ShaderResourceView> m_srv;
		RenderResourcePtr<RHI_UnorderedAccessView> m_uav;

		ColorFormat m_format = ColorFormat::Undefined;
		Vector2u m_size      = {0, 0};

	public:
		RenderSurface();
		RenderSurface& init(ColorFormat format, Vector2i size);
		RenderSurface& release_render_resources() override;

		inline RHI_Texture2D* rhi_texture() const { return m_texture; }
		inline RHI_RenderTargetView* rhi_render_target_view() const { return m_rtv; }
		inline RHI_DepthStencilView* rhi_depth_stencil_view() const { return m_dsv; }
		inline RHI_ShaderResourceView* rhi_shader_resource_view() const { return m_srv; }
		inline RHI_UnorderedAccessView* rhi_unordered_access_view() const { return m_uav; }
		inline Vector2u size() const { return m_size; }
		inline ColorFormat format() const { return m_format; }
	};
}// namespace Engine
