#include <Core/archive.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Graphics/shader.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
	trinex_implement_engine_class_default_init(Shader, 0);

	Shader& Shader::init_render_resources()
	{
		render_thread()->call([this] { m_shader = rhi->create_shader(source_code.data(), source_code.size()); });
		return *this;
	}

	Shader& Shader::release_render_resources()
	{
		Super::release_render_resources();
		m_shader = nullptr;
		return *this;
	}
}// namespace Engine
