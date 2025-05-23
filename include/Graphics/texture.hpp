#pragma once
#include <Core/render_resource.hpp>

struct ImGuiContext;

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
		RenderResourcePtr<RHI_Texture> m_texture;

	public:
		Texture& rhi_bind(byte location);
		Texture& rhi_bind_combined(byte location, RHI_Sampler* sampler);
		Texture& rhi_bind_combined(byte location, Sampler* sampler);
		RHI_ShaderResourceView* rhi_srv() const;
		RHI_Texture* rhi_texture() const;
		virtual TextureType type() const = 0;
	};

}// namespace Engine
