#include <Core/reflection/script_class.hpp>
#include <ScriptEngine/script.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_variable.hpp>
#include <angelscript.h>

namespace Engine::Refl
{
	implement_reflect_type(ScriptClass);

	bool Class::is_script_class(Class* self)
	{
		return instance_cast<ScriptClass>(self) != nullptr;
	}

	ScriptClass::ScriptClass(Class* parent, Script* script, const ScriptTypeInfo& info, BitMask flags)
		: Class(parent, flags | IsScriptable), m_script(script)
	{
		script_type_info = info;
		script->m_refl_objects.insert(this);
	}

	Engine::Object* ScriptClass::object_constructor(StringView name, Engine::Object* owner, bool scriptable)
	{
		trinex_always_check(scriptable, "Cannot create non-scriptable object from scriptable class");
		asITypeInfo* type = script_type_info.info();

		asIScriptFunction* factory = nullptr;
		{
			auto factories = type->GetFactoryCount();
			for (asUINT i = 0; i < factories; ++i)
			{
				auto current_factory = type->GetFactoryByIndex(i);
				if (current_factory->GetParamCount() == 0)
				{
					factory = current_factory;
					break;
				}
			}

			if (factory == nullptr)
			{
				throw EngineException("The script class does not contain a default constructor");
			}
		}

		Engine::Object* obj = nullptr;
		ScriptContext::execute(factory, &obj);

		if (obj == nullptr)
		{
			throw EngineException("Failed to create new instance");
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
