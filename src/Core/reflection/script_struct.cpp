#include <Core/reflection/script_struct.hpp>
#include <ScriptEngine/script.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_variable.hpp>
#include <angelscript.h>

namespace Engine::Refl
{
	trinex_implement_reflect_type(ScriptStruct);

	ScriptStruct::ScriptStruct(ScriptStruct* parent, Script* script, const ScriptTypeInfo& info, BitMask flags)
	    : Struct(parent, flags | IsScriptable), m_script(script)
	{
		script_type_info = info;
		script->m_refl_objects.insert(this);
	}

	void* ScriptStruct::create_struct()
	{
		return nullptr;
	}

	ScriptStruct& ScriptStruct::destroy_struct(void* obj)
	{
		return *this;
	}

	Script* ScriptStruct::script() const
	{
		return m_script;
	}

	size_t ScriptStruct::size() const
	{
		return script_type_info.size();
	}

	ScriptStruct::~ScriptStruct()
	{
		m_script->m_refl_objects.erase(this);
	}
}// namespace Engine::Refl
