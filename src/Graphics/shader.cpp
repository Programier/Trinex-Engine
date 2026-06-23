#include <Core/archive.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Graphics/shader.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	trinex_implement_engine_class_default_init(Shader, 0);

	Shader& Shader::rebuild()
	{
		m_shader = RHI::instance()->create_shader(source, {});
		return *this;
	}
}// namespace Trinex
