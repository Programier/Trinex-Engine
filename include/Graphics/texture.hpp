#pragma once
#include <Core/render_resource.hpp>

struct ImGuiContext;

namespace Engine
{
	struct RHI_Texture;
	struct RHI_ShaderResourceView;
	struct RHI_UnorderedAccessView;

	class ENGINE_EXPORT Texture : public RenderResource
	{
		trinex_declare_class(Texture, RenderResource);

	protected:
		RenderResourcePtr<RHI_Texture> m_texture;
		RenderResourcePtr<RHI_ShaderResourceView> m_srv;
		mutable RenderResourcePtr<RHI_UnorderedAccessView> m_uav;

	public:
		RHI_UnorderedAccessView* rhi_unordered_access_view() const;
		Texture& release_render_resources() override;
		virtual TextureType type() const = 0;

		inline RHI_Texture* rhi_texture() const { return m_texture; }
		inline RHI_ShaderResourceView* rhi_shader_resource_view() const { return m_srv; }
	};

}// namespace Engine
