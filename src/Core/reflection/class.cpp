#include <Core/engine_loading_controllers.hpp>
#include <Core/exception.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <angelscript.h>

namespace Engine::Refl
{
	trinex_implement_reflect_type(Class);

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

	void Class::script_object_constructor(void* object, StringView name, Engine::Object* owner)
	{
		if (!Engine::Object::static_setup_next_object_info())
		{
			int_t type_id = ScriptContext::this_type_id();

			if (Class* script_class = ScriptEngine::find_class(type_id))
				Engine::Object::static_setup_next_object_info(script_class);
		}

		create_placement_object(object, name, owner);
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

				registrar.behave(ScriptClassBehave::Construct, "void f()", &Class::script_object_constructor,
				                 ScriptCallConv::ThisCall_ObjFirst, this);

				registrar.behave(ScriptClassBehave::Construct,
				                 R"(void f(Engine::StringView name = "", Engine::Object owner = null))",
				                 &Class::script_object_constructor_default, ScriptCallConv::ThisCall_ObjFirst, this);

				registrar.behave(ScriptClassBehave::Factory, factory.c_str(), script_object_factory(), ScriptCallConv::CDecl);
			}
		}

		return *this;
	}

	Engine::Object* Class::object_constructor(StringView name, Engine::Object* owner, bool scriptable)
	{
		throw EngineException("Unimplemented method");
		return nullptr;
	}

	Engine::Object* Class::object_placement_constructor(void* mem, StringView name, Engine::Object* owner, bool scriptable)
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

	Engine::Object* Class::create_object(StringView name, Engine::Object* owner)
	{
		if (flags(Class::IsSingletone))
		{
			if (m_singletone_object == nullptr)
			{
				bool scriptable     = !Engine::Object::static_setup_next_object_info(this)->is_native();
				m_singletone_object = object_constructor(name, owner, scriptable);
				m_singletone_object->flags |= Engine::Object::StandAlone;
				m_singletone_object->add_reference();
			}

			return m_singletone_object;
		}

		bool scriptable        = !Engine::Object::static_setup_next_object_info(this)->is_native();
		Engine::Object* object = object_constructor(name, owner, scriptable);
		return object;
	}

	Engine::Object* Class::create_placement_object(void* place, StringView name, Engine::Object* owner)
	{
		if (flags(Class::IsSingletone))
		{
			if (m_singletone_object == nullptr)
			{
				bool scriptable     = !Engine::Object::static_setup_next_object_info(this)->is_native();
				m_singletone_object = object_placement_constructor(place, name, owner, scriptable);
				m_singletone_object->flags |= Engine::Object::StandAlone;
				m_singletone_object->add_reference();
				return m_singletone_object;
			}

			return nullptr;
		}

		bool scriptable        = !Engine::Object::static_setup_next_object_info(this)->is_native();
		Engine::Object* object = object_placement_constructor(place, name, owner, scriptable);
		return object;
	}

	Class& Class::destroy_object(Engine::Object* object)
	{
		trx_delete object;
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

	void Class::register_layout(ScriptClassRegistrar& r, ClassInfo* info, DownCast downcast)
	{
		Super::register_layout(r, info, downcast);
		r.method("Engine::Object@ singletone_instance() const", &Class::singletone_instance);
	}

	static void on_init()
	{
		ScriptClassRegistrar::RefInfo info;
		info.implicit_handle = true;
		info.no_count        = true;

		auto r = ScriptClassRegistrar::reference_class("Engine::Refl::Class", info);
		Class::register_layout(r, Class::static_refl_class_info(), script_downcast<Class>);
		r.method("Class@ parent() const", &Class::parent);
		r.static_function("Class@ static_find(Engine::StringView name, int flags = 0)", Class::static_find<Class>);
		r.static_function("Class@ static_require(StringView name, int flags = 0)", Class::static_require<Class>);
	}

	static ReflectionInitializeController initializer(on_init, "Engine::Refl::Class");
}// namespace Engine::Refl
