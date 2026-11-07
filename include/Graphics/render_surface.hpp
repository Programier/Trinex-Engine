#pragma once
#include <Core/asset.hpp>
#include <Core/math/vector.hpp>
#include <RHI/enums.hpp>

namespace Trinex
{
	class RHIShaderResourceView;
	class RHIUnorderedAccessView;
	class RHIRenderTargetView;
	class RHIDepthStencilView;
	class RHITexture;

	class trinex_api RenderSurface : public Asset
	{
		trinex_class(RenderSurface, Asset);

		Ref<RHITexture> m_texture;

		RHISurfaceFormat m_format = RHISurfaceFormat::Undefined;
		Vector2u m_size           = {0, 0};

	public:
		RenderSurface();
		RenderSurface& init(RHISurfaceFormat format, Vector2i size);
		RenderSurface& rebuild() override;
		RHIRenderTargetView* rtv() const;
		RHIDepthStencilView* dsv() const;
		RHIUnorderedAccessView* uav() const;
		RHIShaderResourceView* srv() const;

		inline RHITexture* handle() const { return m_texture.value(); }
		inline Vector2u size() const { return m_size; }
		inline RHISurfaceFormat format() const { return m_format; }
	};
} // namespace Trinex
