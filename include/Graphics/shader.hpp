#pragma once
#include <Core/enums.hpp>
#include <Graphics/render_resource.hpp>
#include <Graphics/shader_parameters.hpp>

namespace Engine
{
	struct RHIShader;
	class ENGINE_EXPORT Shader : public RenderResource
	{
		trinex_declare_class(Shader, RenderResource);

	protected:
		RHIResourcePtr<RHIShader> m_shader;

	public:
		Buffer source_code;

		Shader& init_render_resources() override;
		Shader& release_render_resources() override;
		inline RHIShader* rhi_shader() const { return m_shader; }
	};
}// namespace Engine
