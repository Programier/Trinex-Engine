#pragma once
#include <Graphics/render_resource.hpp>
#include <RHI/enums.hpp>
#include <RHI/resource_ptr.hpp>

namespace Engine
{
	struct RHI_ShaderResourceView;
	struct RHI_UnorderedAccessView;
	struct RHI_RenderTargetView;
	struct RHI_DepthStencilView;
	struct RHI_Texture;

	class ENGINE_EXPORT RenderSurface : public RenderResource
	{
		trinex_declare_class(RenderSurface, RenderResource);

		RHIResourcePtr<RHI_Texture> m_texture;

		RHISurfaceFormat m_format = RHISurfaceFormat::Undefined;
		Vector2u m_size           = {0, 0};

	public:
		RenderSurface();
		RenderSurface& init(RHISurfaceFormat format, Vector2i size);
		RenderSurface& release_render_resources() override;
		RHI_RenderTargetView* rhi_rtv() const;
		RHI_DepthStencilView* rhi_dsv() const;
		RHI_UnorderedAccessView* rhi_uav() const;
		RHI_ShaderResourceView* rhi_srv() const;

		inline RHI_Texture* rhi_texture() const { return m_texture; }
		inline Vector2u size() const { return m_size; }
		inline RHISurfaceFormat format() const { return m_format; }
	};
}// namespace Engine
