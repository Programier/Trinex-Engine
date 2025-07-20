#pragma once
#include <Graphics/render_resource.hpp>
#include <RHI/resource_ptr.hpp>

namespace Engine
{
	class RHIShaderResourceView;
	class RHISampler;
	class RHITexture;
	class Sampler;

	class ENGINE_EXPORT Texture : public RenderResource
	{
		trinex_declare_class(Texture, RenderResource);

	protected:
		RHIResourcePtr<RHITexture> m_texture;

	public:
		Texture& rhi_bind(byte location);
		Texture& rhi_bind_combined(byte location, RHISampler* sampler);
		Texture& rhi_bind_combined(byte location, Sampler* sampler);
		RHIShaderResourceView* rhi_srv() const;
		RHITexture* rhi_texture() const;
	};

}// namespace Engine
