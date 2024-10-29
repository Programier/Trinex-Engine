#include <Core/reflection/class.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <angelscript.h>

namespace Engine::Refl
{
	static void native_object_constructor(asIScriptObject* object, StringView name, Engine::Object* owner)
	{
		asITypeInfo* type = object->GetObjectType();
		auto* self        = reinterpret_cast<Refl::Class*>(type->GetNativeClassUserData());
		auto* native      = self;

		while (native && !native->is_native()) native = native->parent();

		if (native)
		{
			if (Engine::Object* res = native->create_placement_object(object, name, owner, self))
			{
				res->flags(Refl::Class::IsScriptable, true);
			}
		}
	}

	static void native_default_object_constructor(asIScriptObject* object)
	{
		native_object_constructor(object, "", nullptr);
	}

	static asITypeInfo* get_object_type_info(Engine::Object* object)
	{
		return object->class_instance()->find_valid_script_type_info().info();
	}

	void Class::bind_class_to_script_engine()
	{
		auto registrar = ScriptClassRegistrar::existing_class(full_name());

		auto base = parent();

		while (base && base->script_type_info.info() == nullptr) base = base->parent();

		if (base)
		{
			ScriptEngine::engine()->RegisterObjectBaseType(full_name().c_str(), base->full_name().c_str());
		}

		if (flags(IsConstructible))
		{
			auto factory = Strings::format(R"({}@ f(StringView name = "", Engine::Object owner = null))", full_name());
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
		auto registrar = ScriptClassRegistrar::reference_class(this);

		Class* parent_class = parent();
		while (parent_class && parent_class->script_type_info.info() == nullptr) parent_class = parent_class->parent();

		if (parent_class)
		{
			ReflectionPostInitializeController().push([this]() { bind_class_to_script_engine(); }, full_name(),
													  {parent_class->full_name()});
		}
		else
		{
			ReflectionPostInitializeController().push([this]() { bind_class_to_script_engine(); }, full_name());
		}
	}

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

			asITypeInfo* is_object_ti = ti;
			auto object_ti            = Engine::Object::static_class_instance()->script_type_info.info();

			while (is_object_ti && is_object_ti != object_ti) is_object_ti = is_object_ti->GetBaseType();

			if (is_object_ti)
			{
				self = reinterpret_cast<Class*>(ti->GetNativeClassUserData());
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

		reg.method("Engine::Class@ opImplCast() const", &ClassOf::class_of_impl_cast);
	}

	static PreInitializeController pre_initializer(on_pre_init, "Engine::Class");
	static auto reflection_init = ReflectionPostInitializeController(on_reflection_init);
}// namespace Engine::Refl
