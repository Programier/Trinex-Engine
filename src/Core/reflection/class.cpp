#include <Core/exception.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <angelscript.h>

namespace Engine::Refl
{
	implement_reflect_type(Class);

	bool Struct::is_class() const
	{
		return instance_cast<const Class>(this) != nullptr;
	}

	static FORCE_INLINE Vector<Class*>& get_asset_class_table()
	{
		static Vector<Class*> vector;
		return vector;
	}

	Class::Class(Class* parent, BitMask flags) : Struct(parent, flags)
	{
		info_log("Class", "Created class instance '%s'", this->full_name().c_str());
		m_singletone_object = nullptr;

		if (is_asset())
		{
			get_asset_class_table().push_back(this);
		}
	}

	Class& Class::register_scriptable_instance()
	{
		auto registrar = ScriptClassRegistrar::reference_class(this);
		return *this;
	}

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

	Class& Class::initialize()
	{
		Super::initialize();

		if (is_scriptable() && is_native())
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
				auto factory =
						Strings::format(R"({}@ f(Engine::StringView name = "", Engine::Object owner = null))", full_name());

				registrar.behave(ScriptClassBehave::Construct, "void f()", native_default_object_constructor);
				registrar.behave(ScriptClassBehave::Construct, R"(void f(Engine::StringView name, Engine::Object owner = null))",
								 native_object_constructor);
				registrar.behave(ScriptClassBehave::Factory, factory.c_str(), script_object_factory(), ScriptCallConv::CDecl);
			}

			registrar.behave(ScriptClassBehave::GetTypeInfo, "int& f()", get_object_type_info);
		}

		return *this;
	}

	Engine::Object* Class::object_constructor(Class*, StringView name, Engine::Object* owner)
	{
		throw EngineException("Unimplemented method");
		return nullptr;
	}

	Engine::Object* Class::object_placement_constructor(void* mem, Class* class_overload, StringView name, Engine::Object* owner)
	{
		throw EngineException("Unimplemented method");
		return nullptr;
	}

	Class::ObjectFactory* Class::script_object_factory() const
	{
		throw EngineException("Unimplemented method");
		return nullptr;
	}

	Class* Class::parent() const
	{
		return instance_cast<Class>(Struct::parent());
	}

	void* Class::create_struct()
	{
		return create_object();
	}

	Class& Class::destroy_struct(void* obj)
	{
		return *this;
	}

	Engine::Object* Class::create_object(StringView name, Engine::Object* owner, Class* class_overload)
	{
		if (class_overload)
		{
			trinex_check(class_overload->is_a(this), "Overload class must be instance of this class");
		}
		else
		{
			class_overload = this;
		}

		if (flags(Class::IsSingletone))
		{
			if (m_singletone_object == nullptr)
			{
				Engine::Object::setup_next_object_info(const_cast<Class*>(this));
				m_singletone_object = object_constructor(this, name, owner);
				Engine::Object::reset_next_object_info();
			}

			return m_singletone_object;
		}

		Engine::Object::setup_next_object_info(const_cast<Class*>(this));
		Engine::Object* object = object_constructor(this, name, owner);
		return object;
	}

	Engine::Object* Class::create_placement_object(void* place, StringView name, Engine::Object* owner, Class* class_overload)
	{
		if (class_overload)
		{
			trinex_check(class_overload->is_a(this), "Overload class must be instance of this class");
		}
		else
		{
			class_overload = this;
		}

		if (flags(Class::IsSingletone))
		{
			if (m_singletone_object == nullptr)
			{
				Engine::Object::setup_next_object_info(const_cast<Class*>(class_overload));
				m_singletone_object = object_placement_constructor(place, class_overload, name, owner);
				Engine::Object::reset_next_object_info();
				return m_singletone_object;
			}

			return nullptr;
		}

		Engine::Object::setup_next_object_info(const_cast<Class*>(class_overload));
		Engine::Object* object = object_placement_constructor(place, class_overload, name, owner);
		return object;
	}

	Class& Class::destroy_object(Engine::Object* object)
	{
		delete object;
		return *this;
	}

	const ScriptTypeInfo& Class::find_valid_script_type_info() const
	{
		auto self = this;
		while (self && !self->script_type_info.is_valid()) self = self->parent();
		return self->script_type_info;
	}

	Engine::Object* Class::singletone_instance() const
	{
		return m_singletone_object;
	}

	const Vector<Class*>& Class::asset_classes()
	{
		return get_asset_class_table();
	}

	static Class* static_find_class(const StringView& name)
	{
		return Class::static_find(name);
	}

	static bool static_is_a(Class* self, Class* obj)
	{
		return self->is_a(obj);
	}

	static void on_init()
	{
		ScriptClassRegistrar::RefInfo info;
		info.implicit_handle = true;
		info.no_count        = true;

		auto reg               = ScriptClassRegistrar::reference_class("Engine::Refl::Class", info);
		auto refl              = Refl::Object::new_instance<NativeStruct<Class>>("Engine::Refl::Class");
		refl->script_type_info = reg.type_info();

		refl->on_initialize += [](Refl::Object* self) {
			auto reg = ScriptClassRegistrar::existing_class("Engine::Refl::Class");
			reg.method("Class@ parent() const", &Class::parent);
			reg.method("string full_name() const", method_of<String>(&Class::full_name));
			reg.method("const Name& namespace_name() const", &Class::scope_name);
			reg.method("const Name& name() const", &Class::name);
			reg.method("bool is_scriptable() const", &Class::is_scriptable);
			reg.method("Object@ singletone_instance() const", &Class::singletone_instance);
			reg.method("bool is_asset() const", &Class::is_asset);
			reg.method("bool is_native() const", &Class::is_native);
			reg.static_function("Class@ static_find(const StringView& in)", static_find_class);
			reg.method("bool is_a(const Class@) const", static_is_a);
			reg.method("uint64 size() const", &Class::size);
		};
	}

	static ReflectionInitializeController initializer(on_init, "Engine::Refl::Class");
}// namespace Engine::Refl
