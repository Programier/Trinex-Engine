#include <Core/class.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <angelscript.h>

namespace Engine
{
	static void native_object_constructor(asIScriptObject* object, StringView name, Object* owner)
	{
		asITypeInfo* type = object->GetObjectType();
		Class* self		  = reinterpret_cast<Class*>(type->GetNativeClassUserData());
		Class* native	  = self;

		while (native && !native->is_native()) native = native->parent();

		if (native)
		{
			if (Object* res = native->create_placement_object(object, name, owner, self))
			{
				res->flags(Class::IsScriptable, true);
			}
		}
	}

	static void native_default_object_constructor(asIScriptObject* object)
	{
		native_object_constructor(object, "", nullptr);
	}

	static asITypeInfo* get_object_type_info(Object* object)
	{
		return object->class_instance()->script_type_info.info();
	}

	void Class::bind_class_to_script_engine()
	{
		auto registrar = ScriptClassRegistrar::existing_class(name().to_string());

		if (auto base = parent())
		{
			ScriptEngine::engine()->RegisterObjectBaseType(name().c_str(), base->name().c_str());
		}

		if (flags(IsConstructible))
		{
			auto factory = Strings::format(R"({}@ f(StringView name = "", Engine::Object owner = null))", name().to_string());
			registrar.behave(ScriptClassBehave::Construct, "void f()", native_default_object_constructor);
			registrar.behave(ScriptClassBehave::Construct, R"(void f(StringView name, Engine::Object owner = null))",
							 native_object_constructor);
			registrar.behave(ScriptClassBehave::Factory, factory.c_str(), m_script_factory, ScriptCallConv::CDecl);
		}

		registrar.behave(ScriptClassBehave::GetTypeInfo, "int& f()", get_object_type_info);

		if (script_registration_callback)
		{
			script_registration_callback(&registrar, this);
		}
	}

	void Class::register_scriptable_class()
	{
		auto registrar		= ScriptClassRegistrar::reference_class(this);
		Class* parent_class = parent();

		if (parent_class)
		{
			ScriptBindingsInitializeController().push([this]() { bind_class_to_script_engine(); }, name().to_string(),
													  {parent_class->name().to_string()});
		}
		else
		{
			ScriptBindingsInitializeController().push([this]() { bind_class_to_script_engine(); }, name().to_string());
		}
	}


	static void on_script_engine_terminate()
	{
		for (auto& [key, value] : Struct::struct_map())
		{
			if (value && value->is_class())
			{
				Class* class_instance = reinterpret_cast<Class*>(value);
				class_instance->script_type_info.release();
			}
		}
	}

	static void on_pre_init()
	{
		ScriptEngine::on_terminate.push(on_script_engine_terminate);
	}

	static PreInitializeController pre_initializer(on_pre_init, "Engine::Class");
}// namespace Engine
