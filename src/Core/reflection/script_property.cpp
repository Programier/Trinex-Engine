#include <Core/reflection/script_property.hpp>
#include <Core/reflection/script_struct.hpp>
#include <ScriptEngine/script_type_info.hpp>

namespace Engine::Refl
{
	size_t ScriptEnumProperty::size() const
	{
		return sizeof(int);
	}

	ScriptObjectProperty::ScriptObjectProperty(uint_t offset, Class* instance) : ScriptProperty(offset), m_instance(instance) {}

	Class* ScriptObjectProperty::class_instance() const
	{
		return m_instance;
	}

	ScriptStructProperty::ScriptStructProperty(uint_t offset, Struct* instance) : ScriptProperty(offset), m_instance(instance) {}

	Struct* ScriptStructProperty::struct_instance() const
	{
		return m_instance;
	}

	size_t ScriptStructProperty::size() const
	{
		return m_instance->size();
	}
}// namespace Engine::Refl
