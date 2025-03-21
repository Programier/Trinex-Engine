#pragma once
#include <Core/render_resource.hpp>

struct ImGuiContext;

namespace Engine
{
	struct RHI_ShaderResourceView;
	struct RHI_Sampler;
	class Sampler;

	class ENGINE_EXPORT Texture : public RenderResource
	{
		trinex_declare_class(Texture, RenderResource);

	protected:
		RenderResourcePtr<RHI_ShaderResourceView> m_srv;

	public:
		Texture& rhi_bind(byte location);
		Texture& rhi_bind_combined(byte location, RHI_Sampler* sampler);
		Texture& rhi_bind_combined(byte location, Sampler* sampler);
		Texture& release_render_resources() override;
		virtual TextureType type() const = 0;

		inline RHI_ShaderResourceView* rhi_shader_resource_view() const { return m_srv; }
	};

}// namespace Engine
