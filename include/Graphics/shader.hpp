#pragma once
#include <Core/enums.hpp>
#include <Graphics/render_resource.hpp>
#include <Graphics/shader_parameters.hpp>

namespace Engine
{
	struct RHI_Shader;
	class ENGINE_EXPORT Shader : public RenderResource
	{
		trinex_declare_class(Shader, RenderResource);

	protected:
		RHIResourcePtr<RHI_Shader> m_shader;

	public:
		Buffer source_code;

		Shader& init_render_resources() override;
		Shader& release_render_resources() override;
		inline RHI_Shader* rhi_shader() const { return m_shader; }
	};
}// namespace Engine
