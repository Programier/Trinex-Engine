#include <Core/logger.hpp>
#include <Core/reflection/script_class.hpp>
#include <ScriptEngine/script.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_variable.hpp>
#include <angelscript.h>

namespace Engine::Refl
{
	trinex_implement_reflect_type(ScriptClass);

	bool Class::is_script_class(Class* self)
	{
		return instance_cast<ScriptClass>(self) != nullptr;
	}

	ScriptClass::ScriptClass(Class* parent, Script* script, const ScriptTypeInfo& info, BitMask flags)
	    : Class(parent, flags | IsScriptable), m_script(script)
	{
		ScriptEngine::register_class(info.type_id(), this);

		script_type_info = info;
		script->m_refl_objects.insert(this);

		auto factories = info.factory_count();


		for (uint_t i = 0; i < factories; ++i)
		{
			m_factory = info.factory_by_index(i);

			if (m_factory.param_count() == 0)
			{
				break;
			}
		}

		trinex_verify_msg(m_factory.is_valid(), "The script class does not contain a default constructor");
	}

	Engine::Object* ScriptClass::object_constructor(StringView name, Engine::Object* owner, bool scriptable)
	{
		trinex_verify_msg(scriptable, "Cannot create non-scriptable object from scriptable class");

		Engine::Object* obj = nullptr;
		ScriptContext::execute(m_factory, &obj);

		if (obj == nullptr)
		{
			error_log("ScriptClass", "Failed to create new instance");
			return nullptr;
		}

		if (!name.empty() || owner)
		{
			obj->rename(name.empty() ? obj->name().to_string() : name, owner);
		}

		return obj;
	}

	ScriptClass& ScriptClass::destroy_object(Engine::Object* object)
	{
		auto script_object = reinterpret_cast<asIScriptObject*>(object);
		script_object->Destroy();
		return *this;
	}

	Script* ScriptClass::script() const
	{
		return m_script;
	}

	size_t ScriptClass::size() const
	{
		return script_type_info.size();
	}

	ScriptClass::~ScriptClass()
	{

		m_script->m_refl_objects.erase(this);
	}
}// namespace Engine::Refl
