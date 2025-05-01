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

	public:
		Texture& rhi_bind(byte location);
		Texture& rhi_bind_combined(byte location, RHI_Sampler* sampler);
		Texture& rhi_bind_combined(byte location, Sampler* sampler);
		virtual TextureType type() const                = 0;
		virtual RHI_ShaderResourceView* rhi_srv() const = 0;
	};

}// namespace Engine
