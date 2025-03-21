#pragma once
#include <Core/object.hpp>
#include <Core/render_resource_ptr.hpp>

namespace Engine
{
	struct RHI_Object;
	struct RHI_BindingObject;
	struct RHI_Sampler;
	struct RHI_RenderTarget;
	struct RHI_Texture;
	struct RHI_Shader;
	struct RHI_Pipeline;
	struct RHI_Buffer;
	struct RHI_VertexBuffer;
	struct RHI_IndexBuffer;
	struct RHI_UniformBuffer;
	struct RHI_SSBO;
	struct RHI_RenderPass;

	class ENGINE_EXPORT RenderResource : public Object
	{
		trinex_declare_class(RenderResource, Object);

	public:
		virtual RenderResource& init_render_resources();
		virtual RenderResource& release_render_resources();

		RenderResource& on_destroy() override;
		RenderResource& postload() override;
	};
}// namespace Engine
