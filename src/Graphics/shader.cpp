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

	VertexShader& VertexShader::init_render_resources()
	{
		render_thread()->call([this] {
			m_shader = rhi->create_vertex_shader(source_code.data(), source_code.size(), attributes.data(), attributes.size());
		});
		return *this;
	}

	FragmentShader& FragmentShader::init_render_resources()
	{
		render_thread()->call([this] { m_shader = rhi->create_fragment_shader(source_code.data(), source_code.size()); });
		return *this;
	}

	TessellationControlShader& TessellationControlShader::init_render_resources()
	{
		render_thread()->call(
		        [this] { m_shader = rhi->create_tesselation_control_shader(source_code.data(), source_code.size()); });
		return *this;
	}

	TessellationShader& TessellationShader::init_render_resources()
	{
		render_thread()->call([this] { m_shader = rhi->create_tesselation_shader(source_code.data(), source_code.size()); });
		return *this;
	}

	GeometryShader& GeometryShader::init_render_resources()
	{
		render_thread()->call([this] { m_shader = rhi->create_geometry_shader(source_code.data(), source_code.size()); });
		return *this;
	}

	ComputeShader& ComputeShader::init_render_resources()
	{
		render_thread()->call([this] { m_shader = rhi->create_compute_shader(source_code.data(), source_code.size()); });
		return *this;
	}
}// namespace Engine
