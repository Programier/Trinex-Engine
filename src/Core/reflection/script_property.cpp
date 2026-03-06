#include <Core/reflection/script_property.hpp>
#include <Core/reflection/script_struct.hpp>
#include <ScriptEngine/script_type_info.hpp>

namespace Engine::Refl
{
	usize ScriptEnumProperty::size() const
	{
		return sizeof(int);
	}

	ScriptObjectProperty::ScriptObjectProperty(u32 offset, Class* instance) : ScriptProperty(offset), m_instance(instance) {}

	Class* ScriptObjectProperty::class_instance() const
	{
		return m_instance;
	}

	ScriptStructProperty::ScriptStructProperty(u32 offset, Struct* instance) : ScriptProperty(offset), m_instance(instance) {}

	Struct* ScriptStructProperty::struct_instance() const
	{
		return m_instance;
	}

	usize ScriptStructProperty::size() const
	{
		return m_instance->size();
	}
}// namespace Engine::Refl
