#pragma once
#include <Graphics/render_resource.hpp>
#include <RHI/resource_ptr.hpp>

namespace Engine
{
	struct RHI_ShaderResourceView;
	struct RHI_Sampler;
	struct RHI_Texture;
	class Sampler;

	class ENGINE_EXPORT Texture : public RenderResource
	{
		trinex_declare_class(Texture, RenderResource);

	protected:
		RHIResourcePtr<RHI_Texture> m_texture;

	public:
		Texture& rhi_bind(byte location);
		Texture& rhi_bind_combined(byte location, RHI_Sampler* sampler);
		Texture& rhi_bind_combined(byte location, Sampler* sampler);
		RHI_ShaderResourceView* rhi_srv() const;
		RHI_Texture* rhi_texture() const;
	};

}// namespace Engine
