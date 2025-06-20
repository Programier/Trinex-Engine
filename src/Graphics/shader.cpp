#include <Core/archive.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Graphics/shader.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
	trinex_implement_engine_class_default_init(Shader, 0);
	trinex_implement_engine_class_default_init(VertexShader, 0);

	trinex_implement_engine_class_default_init(TessellationControlShader, 0);
	trinex_implement_engine_class_default_init(TessellationShader, 0);
	trinex_implement_engine_class_default_init(GeometryShader, 0);
	trinex_implement_engine_class_default_init(FragmentShader, 0);
	trinex_implement_engine_class_default_init(ComputeShader, 0);

	Shader& Shader::release_render_resources()
	{
		Super::release_render_resources();
		m_shader = nullptr;
		return *this;
	}

	bool Shader::serialize(Archive& ar)
	{
		if (!Super::serialize(ar))
			return false;
		return ar;
	}

	bool Shader::serialize_source_code(Archive& ar)
	{
		return ar.serialize(source_code);
	}

	VertexShader& VertexShader::init_render_resources()
	{
		render_thread()->call([this] {
			m_shader = rhi->create_vertex_shader(source_code.data(), source_code.size(), attributes.data(), attributes.size());
		});
		return *this;
	}

	bool VertexShader::serialize(Archive& ar)
	{
		if (!Super::serialize(ar))
			return false;
		return ar;
	}

	ShaderType VertexShader::type() const
	{
		return ShaderType::Vertex;
	}

	FragmentShader& FragmentShader::init_render_resources()
	{
		render_thread()->call([this] { m_shader = rhi->create_fragment_shader(source_code.data(), source_code.size()); });
		return *this;
	}

	ShaderType FragmentShader::type() const
	{
		return ShaderType::Fragment;
	}

	TessellationControlShader& TessellationControlShader::init_render_resources()
	{
		render_thread()->call(
		        [this] { m_shader = rhi->create_tesselation_control_shader(source_code.data(), source_code.size()); });
		return *this;
	}

	ShaderType TessellationControlShader::type() const
	{
		return ShaderType::TessellationControl;
	}

	TessellationShader& TessellationShader::init_render_resources()
	{
		render_thread()->call([this] { m_shader = rhi->create_tesselation_shader(source_code.data(), source_code.size()); });
		return *this;
	}

	ShaderType TessellationShader::type() const
	{
		return ShaderType::Tessellation;
	}

	GeometryShader& GeometryShader::init_render_resources()
	{
		render_thread()->call([this] { m_shader = rhi->create_geometry_shader(source_code.data(), source_code.size()); });
		return *this;
	}

	ShaderType GeometryShader::type() const
	{
		return ShaderType::Geometry;
	}

	ComputeShader& ComputeShader::init_render_resources()
	{
		render_thread()->call([this] { m_shader = rhi->create_compute_shader(source_code.data(), source_code.size()); });
		return *this;
	}

	ShaderType ComputeShader::type() const
	{
		return ShaderType::Compute;
	}
}// namespace Engine
