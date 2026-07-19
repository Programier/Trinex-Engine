#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/script_binding.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <angelscript.h>

namespace Trinex::Refl
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

	trinex_on_pre_init()
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
			auto target          = Trinex::Object::static_reflection()->script_type_info.info();

			while (current && current != target) current = current->GetBaseType();

			if (current)
			{
				String fullname = Strings::concat_scoped_name(ti->GetNamespace(), ti->GetName());
				self            = Refl::Class::static_find(fullname);
			}
		}

		ClassOf(asITypeInfo* ti, const ClassOf& other) { self = other.self; }

		Class* class_of_impl_cast() const { return self; }
	};

	trinex_on_reflection_init({.after = {"Trinex::Refl::Class"}})
	{
		auto reg = ScriptBinding::Class::create("Trinex::class_of<T>",
		                                        ScriptBinding::value_type<ClassOf>(ScriptClassFlags::Template));
		reg.behaviour(ScriptClassBehave::Construct, "void f(int&)", ScriptBinding::Helpers::constructor<ClassOf, asITypeInfo*>);
		reg.behaviour(ScriptClassBehave::Construct, "void f(int&, const class_of<T>& in other)",
		              ScriptBinding::Helpers::constructor<ClassOf, asITypeInfo*, const ClassOf&>);
		reg.behaviour(ScriptClassBehave::Destruct, "void f()", ScriptBinding::Helpers::destructor<ClassOf>);

		reg.method("Trinex::Refl::Class@ opImplCast() const", &ClassOf::class_of_impl_cast);
	}
}// namespace Trinex::Refl
