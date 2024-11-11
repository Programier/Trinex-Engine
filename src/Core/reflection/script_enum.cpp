#include <Core/logger.hpp>
#include <Core/reflection/script_enum.hpp>
#include <ScriptEngine/script.hpp>

namespace Engine::Refl
{
	implement_reflect_type(ScriptEnum);

	ScriptEnum::ScriptEnum(Script* script, const ScriptTypeInfo& info) : m_script(script)
	{
		m_info      = info;
		String name = full_name();

		auto count = info.enum_value_count();
		for (uint_t i = 0; i < count; ++i)
		{
			int_t value;
			StringView name = info.enum_value_by_index(i, &value);
			create_entry(nullptr, name, value);
		}

		script->m_refl_objects.insert(this);
	}

	Script* ScriptEnum::script() const
	{
		return m_script;
	}

	ScriptEnum::~ScriptEnum()
	{
		m_script->m_refl_objects.erase(this);
	}
}// namespace Engine::Refl
