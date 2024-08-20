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
			native->create_placement_object(object, name, owner, self);
		}
	}

	static void native_default_object_constructor(asIScriptObject* object)
	{
		native_object_constructor(object, "", nullptr);
	}

	static ScriptClassRegistrar registrar_of(Class* self, bool exiting)
	{
		if (exiting)
		{
			return ScriptClassRegistrar::existing_class(self->name().to_string());
		}

		ScriptClassRegistrar::RefInfo info;
		info.no_count		 = true;
		info.implicit_handle = true;
		info.extra_flags	 = asOBJ_APP_NATIVE_INHERITANCE;

		return ScriptClassRegistrar::reference_class(self->name().to_string(), info, self->sizeof_class());
	}

	void Class::bind_class_to_script_engine()
	{
		ScriptClassRegistrar registrar = registrar_of(this, true);

		if (flags(IsConstructible))
		{
			auto factory = Strings::format(R"({}@ f(StringView name = "", Engine::Object owner = null))", name().to_string());
			registrar.behave(ScriptClassBehave::Construct, "void f()", native_default_object_constructor);
			registrar.behave(ScriptClassBehave::Construct, R"(void f(StringView name, Engine::Object owner = null))",
							 native_object_constructor);
			registrar.behave(ScriptClassBehave::Factory, factory.c_str(), m_script_factory, ScriptCallConv::CDecl);
		}

		registrar.type_info().info()->SetNativeClassUserData(this);

		Class* current = this;
		List<Class*> stack;

		while (current)
		{
			stack.push_back(current);
			current = current->parent();
		}

		while (!stack.empty())
		{
			current = stack.back();
			if (current->script_registration_callback)
			{
				current->script_registration_callback(&registrar, this);
			}

			stack.pop_back();
		}
	}

	void Class::register_scriptable_class()
	{
		ScriptClassRegistrar registrar = registrar_of(this, false);
		script_type_info			   = registrar.type_info();
		ScriptBindingsInitializeController().push([this]() { bind_class_to_script_engine(); });
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
