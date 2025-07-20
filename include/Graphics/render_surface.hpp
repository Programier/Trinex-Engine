#pragma once
#include <Graphics/render_resource.hpp>
#include <RHI/enums.hpp>
#include <RHI/resource_ptr.hpp>

namespace Engine
{
	struct RHIShaderResourceView;
	struct RHIUnorderedAccessView;
	struct RHIRenderTargetView;
	struct RHIDepthStencilView;
	struct RHITexture;

	class ENGINE_EXPORT RenderSurface : public RenderResource
	{
		trinex_declare_class(RenderSurface, RenderResource);

		RHIResourcePtr<RHITexture> m_texture;

		RHISurfaceFormat m_format = RHISurfaceFormat::Undefined;
		Vector2u m_size           = {0, 0};

	public:
		RenderSurface();
		RenderSurface& init(RHISurfaceFormat format, Vector2i size);
		RenderSurface& release_render_resources() override;
		RHIRenderTargetView* rhi_rtv() const;
		RHIDepthStencilView* rhi_dsv() const;
		RHIUnorderedAccessView* rhi_uav() const;
		RHIShaderResourceView* rhi_srv() const;

		inline RHITexture* rhi_texture() const { return m_texture; }
		inline Vector2u size() const { return m_size; }
		inline RHISurfaceFormat format() const { return m_format; }
	};
}// namespace Engine
