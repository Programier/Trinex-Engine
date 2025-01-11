#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <angelscript.h>

namespace Engine::Refl
{
	static void visit_script_classes(Object* object)
	{
		if (auto scope = Object::instance_cast<ScopedType>(object))
		{
			for (auto& [name, child] : scope->childs())
			{
				visit_script_classes(child);
			}

			if (auto class_instance = Object::instance_cast<Class>(scope))
			{
				class_instance->script_type_info.release();
			}
		}
	}

	static void on_script_engine_terminate()
	{
		visit_script_classes(Object::static_root());
	}

	static void on_pre_init()
	{
		ScriptEngine::on_terminate.push(on_script_engine_terminate);
	}

	struct ClassOf {
		Class* self = nullptr;

		ClassOf(asITypeInfo* ti)
		{
			auto sub_type_id = ti->GetSubTypeId();

			if (!ScriptEngine::is_object_type(sub_type_id, true))
				return;

			ti = ti->GetSubType(0);

			asITypeInfo* current = ti;
			auto target          = Engine::Object::static_class_instance()->script_type_info.info();

			while (current && current != target) current = current->GetBaseType();

			if (current)
			{
				String fullname = Strings::concat_scoped_name(ti->GetNamespace(), ti->GetName());
				self            = Refl::Class::static_find(fullname);
			}
		}

		ClassOf(asITypeInfo* ti, const ClassOf& other)
		{
			self = other.self;
		}

		Class* class_of_impl_cast() const
		{
			return self;
		}
	};

	static void on_reflection_init()
	{
		ScriptClassRegistrar::ValueInfo info;
		info.template_type        = "<T>";
		info.pod                  = false;
		info.has_constructor      = true;
		info.has_copy_constructor = true;
		info.has_destructor       = true;

		auto reg = ScriptClassRegistrar::value_class("Engine::class_of<class T>", sizeof(ClassOf), info);
		reg.behave(ScriptClassBehave::Construct, "void f(int&)", ScriptClassRegistrar::constructor<ClassOf, asITypeInfo*>);
		reg.behave(ScriptClassBehave::Construct, "void f(int&, const class_of<T>& in other)",
				   ScriptClassRegistrar::constructor<ClassOf, asITypeInfo*, const ClassOf&>);
		reg.behave(ScriptClassBehave::Destruct, "void f()", ScriptClassRegistrar::destructor<ClassOf>);

		reg.method("Engine::Refl::Class@ opImplCast() const", &ClassOf::class_of_impl_cast);
	}

	static PreInitializeController pre_init(on_pre_init);
	static ReflectionInitializeController reflection_init(on_reflection_init, "Engine::class_of", {"Engine::Refl::Class"});
}// namespace Engine::Refl
