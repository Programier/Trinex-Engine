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
		
		Shader& release_render_resources() override;
		inline RHI_Shader* rhi_shader() const { return m_shader; }
	};

	class ENGINE_EXPORT VertexShader : public Shader
	{
		trinex_declare_class(VertexShader, Shader);

	public:
		Vector<struct RHIVertexAttribute> attributes;

	public:
		VertexShader& init_render_resources() override;
	};

	class ENGINE_EXPORT FragmentShader : public Shader
	{
		trinex_declare_class(FragmentShader, Shader);

	public:
		FragmentShader& init_render_resources() override;
	};

	class ENGINE_EXPORT TessellationControlShader : public Shader
	{
		trinex_declare_class(TessellationControlShader, Shader);

	public:
		TessellationControlShader& init_render_resources() override;
	};

	class ENGINE_EXPORT TessellationShader : public Shader
	{
		trinex_declare_class(TessellationShader, Shader);

	public:
		TessellationShader& init_render_resources() override;
	};

	class ENGINE_EXPORT GeometryShader : public Shader
	{
		trinex_declare_class(GeometryShader, Shader);

	public:
		GeometryShader& init_render_resources() override;
	};

	class ENGINE_EXPORT ComputeShader : public Shader
	{
		trinex_declare_class(ComputeShader, Shader);

	public:
		ComputeShader& init_render_resources() override;
	};
}// namespace Engine
