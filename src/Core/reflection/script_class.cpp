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

	ScriptClass::ScriptClass(Class* parent, Script* script, BitMask flags) : Class(parent, flags), m_script(script)
	{
		script->m_classes.insert(this);
	}

	Engine::Object* ScriptClass::object_constructor(Class* class_overload, StringView name, Engine::Object* owner)
	{
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

		auto obj = ScriptContext::execute(factory).address_as<Engine::Object>();

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
		std::destroy_at(object);

		script_object->FreeObjectMemory();
		return *this;
	}

	ScriptClass::~ScriptClass()
	{
		m_script->m_classes.erase(this);
	}
}// namespace Engine::Refl
