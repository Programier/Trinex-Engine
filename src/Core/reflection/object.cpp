#include <Core/constants.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/set.hpp>
#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/reflection/object.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>

namespace Engine::Refl
{
	namespace Meta
	{
		ENGINE_EXPORT Name display_name = "display_name";
		ENGINE_EXPORT Name tooltip      = "tooltip";
		ENGINE_EXPORT Name description  = "description";
		ENGINE_EXPORT Name group        = "group";
		ENGINE_EXPORT Name renderer     = "renderer";
	}// namespace Meta

	static Set<Object*> m_instances;
	static bool m_check_exiting_instance = true;

	static bool m_has_next_object_info   = false;
	static StringView m_next_object_name = "";
	static Object* m_next_object_owner   = nullptr;

	static void destroy_reflection_instances()
	{
		m_check_exiting_instance = false;

		while (!m_instances.empty())
		{
			Object::destroy_instance(*m_instances.begin());
		}

		m_check_exiting_instance = true;
	}

	static PostDestroyController destroy_controller(destroy_reflection_instances);

	ClassInfo::ClassInfo(const char* name, const ClassInfo* const parent)
		: class_name(Strings::class_name_sv_of(name)), parent(parent), is_scriptable(false)
	{}

	bool ClassInfo::is_a(const ClassInfo* const info) const
	{
		const ClassInfo* self = this;
		while (self && self != info) self = self->parent;
		return self != nullptr;
	}

	ClassInfo* Object::static_refl_class_info()
	{
		static ClassInfo info("Object", nullptr);
		return &info;
	}

	String Object::concat_scoped_name(StringView scope, StringView name)
	{
		if (scope == static_root()->m_name)
			return String(name);
		return Strings::concat_scoped_name(scope, name);
	}

	void Object::initialize_next_object(StringView name)
	{
		trinex_always_check(!name.empty(), "Reflection object name cannot be empty!");

		if (name != "Root")
		{
			trinex_always_check(static_find(name, FindFlags::DisableReflectionCheck) == nullptr,
								"Object with same name already exist");
		}

		auto owner_name = Strings::namespace_sv_of(name);

		if (owner_name.empty())
		{
			if (name != "Root")
			{
				m_next_object_owner = static_root();
			}
		}
		else
		{
			m_next_object_owner = static_find(owner_name, FindFlags::CreateScope);
		}

		m_next_object_name     = Strings::class_name_sv_of(name);
		m_has_next_object_info = true;
	}

	void Object::initialize_next_object(Object* owner, StringView name)
	{
		trinex_always_check(!name.empty(), "Reflection object name cannot be empty!");

		if (owner)
		{
			trinex_always_check(owner->find(name, FindFlags::DisableReflectionCheck) == nullptr,
								"Object with same name already exist");
		}

		m_next_object_name     = Strings::class_name_sv_of(name);
		m_next_object_owner    = owner;
		m_has_next_object_info = true;
	}

	void Object::setup_owner()
	{
		if (m_next_object_owner)
		{
			owner(m_next_object_owner);
			m_next_object_owner = nullptr;
		}

		m_has_next_object_info = false;
	}

	Object::Object() : m_owner(nullptr), m_name(m_next_object_name)
	{
		trinex_always_check(m_has_next_object_info, "Use new_instance or new_child method for creating reflection objects!");
		m_instances.insert(this);
		m_next_object_name = "";

		display_name(Strings::make_sentence(name().to_string()));
	}

	Object::~Object()
	{
		m_instances.erase(this);

		if (m_metadata)
		{
			delete m_metadata;
		}
	}

	ClassInfo* Object::refl_class_info() const
	{
		return This::static_refl_class_info();
	}

	Object& Object::unregister_subobject(Object* subobject)
	{
		throw EngineException("Cannot unregister object to non-scoped object");
		return *this;
	}

	Object& Object::register_subobject(Object* subobject)
	{
		throw EngineException("Cannot register object to non-scoped object");
		return *this;
	}

	Object& Object::initialize()
	{
		return *this;
	}

	Object& Object::construct()
	{
		return *this;
	}

	Object& Object::owner(Object* object)
	{
		if (m_owner)
		{
			m_owner->unregister_subobject(this);
			m_owner = nullptr;
		}

		if (object)
		{
			m_owner = object;
			object->register_subobject(this);
		}

		return *this;
	}

	Object* Object::owner() const
	{
		return m_owner;
	}

	const Name& Object::name() const
	{
		return m_name;
	}

	static inline const String& string_meta(const Any& any)
	{
		if (any.is_a<String>())
			return any.cast<const String&>();
		return default_value_of<String>();
	}

	static inline const String& string_meta(const Any& any, const String& default_value)
	{
		if (any.is_a<String>())
			return any.cast<const String&>();
		return default_value;
	}

	const String& Object::display_name() const
	{
		if (auto* meta = find_metadata(Meta::display_name))
			return string_meta(*meta, m_name.to_string());

		return m_name.to_string();
	}

	const String& Object::tooltip() const
	{
		return string_meta(metadata(Meta::tooltip));
	}

	const String& Object::description() const
	{
		if (auto* meta = find_metadata(Meta::description))
			return string_meta(*meta, m_name.to_string());

		return m_name.to_string();
	}

	const String& Object::group() const
	{
		return string_meta(metadata(Meta::group));
	}

	Object& Object::display_name(StringView name)
	{
		return metadata(Meta::display_name, String(name));
	}

	Object& Object::tooltip(StringView text)
	{
		return metadata(Meta::tooltip, String(text));
	}

	Object& Object::description(StringView text)
	{
		return metadata(Meta::description, String(text));
	}

	Object& Object::group(StringView text)
	{
		return metadata(Meta::group, String(text));
	}

	void Object::full_name(String& out) const
	{
		if (m_owner && m_owner != static_root())
		{
			m_owner->full_name(out);
		}

		if (!out.empty())
		{
			out += Constants::name_separator;
		}

		out += m_name;
	}

	const Any* Object::find_metadata(const Name& name) const
	{
		if (m_metadata)
		{
			auto it = m_metadata->find(name);
			if (it == m_metadata->end())
				return nullptr;
			return &it->second;
		}

		return nullptr;
	}

	const Any& Object::metadata(const Name& name) const
	{
		if (auto meta = find_metadata(name))
		{
			return *meta;
		}
		return default_value_of<Any>();
	}

	Object& Object::metadata(const Name& name, const Any& any)
	{
		if (m_metadata == nullptr)
		{
			m_metadata = new MetaData();
		}

		(*m_metadata)[name] = any;
		return *this;
	}

	Object& Object::remove_metadata(const Name& name)
	{
		if (m_metadata)
		{
			m_metadata->erase(name);
		}
		return *this;
	}

	String Object::full_name() const
	{
		String result;
		result.reserve(64);
		full_name(result);
		return result;
	}

	String Object::scope_name() const
	{
		if (m_owner && m_owner != static_root())
		{
			return m_owner->full_name();
		}

		return "";
	}

	bool Object::is_initialized() const
	{
		return m_is_initialized;
	}

	Object* Object::find(StringView name, FindFlags flags)
	{
		if ((flags & FindFlags::CreateScope) == FindFlags::CreateScope)
		{
			throw EngineException("Cannot create new scope with non-scoped owner");
		}

		if ((flags & FindFlags::IsRequired) == FindFlags::IsRequired)
		{
			throw EngineException(Strings::format("Failed to find reflection for '{}'", concat_scoped_name(full_name(), name)));
		}

		return nullptr;
	}

	Object* Object::static_find(StringView name, FindFlags flags)
	{
		return static_root()->find(name, flags);
	}

	Object* Object::static_require(StringView name, FindFlags flags)
	{
		return static_find(name, flags | FindFlags::IsRequired);
	}

	bool Object::destroy_instance(Object* object)
	{
		if (!m_check_exiting_instance || m_instances.find(object) != m_instances.end())
		{
			bool check_exiting_instance_tmp = m_check_exiting_instance;
			m_check_exiting_instance        = true;

			if (object->m_owner)
			{
				object->m_owner->unregister_subobject(object);
			}

			delete object;

			m_check_exiting_instance = check_exiting_instance_tmp;
			return true;
		}
		return false;
	}

	bool Object::is_valid(Object* object)
	{
		return m_instances.contains(object);
	}

	static Object* self_address(Object* object)
	{
		return object;
	}

	void Object::register_layout(ScriptClassRegistrar& r, ClassInfo* info, DownCast downcast)
	{
		using T = Object;
		ReflectionInitializeController().require("Engine::Name");

		info->is_scriptable = true;

		for (auto i = info->parent; i; i = i->parent)
		{
			if (!i->is_scriptable)
			{
				ReflectionInitializeController().require(Strings::format("Engine::Refl::{}", i->class_name.to_string()));
			}
		}

		r.method("Engine::Refl::Object@ owner(Engine::Refl::Object@ new_owner)", method_of<Object&>(&T::owner));
		r.method("Engine::Refl::Object@ owner()", method_of<Object*>(&T::owner));
		r.method("const Engine::Name& name() const", &T::name);
		r.method("string full_name() const", method_of<String>(&T::full_name));
		r.method("string scope_name() const", &T::scope_name);
		r.method("bool is_initialized() const", &T::is_initialized);

		r.method("const string& display_name() const", method_of<const String&>(&T::display_name));
		r.method("const string& tooltip() const", method_of<const String&>(&T::tooltip));
		r.method("const string& description() const", method_of<const String&>(&T::description));
		r.method("const string& group() const", method_of<const String&>(&T::group));

		r.method("Engine::Refl::Object@ display_name(Engine::StringView name)", method_of<Object&>(&T::display_name));
		r.method("Engine::Refl::Object@ tooltip(Engine::StringView tooltip)", method_of<Object&>(&T::tooltip));
		r.method("Engine::Refl::Object@ description(Engine::StringView description)", method_of<Object&>(&T::description));
		r.method("Engine::Refl::Object@ group(Engine::StringView group)", method_of<Object&>(&T::group));

		r.method("Engine::Refl::Object@ remove_metadata(const Engine::Name& name)", &T::remove_metadata);

		r.method("Engine::Refl::Object@ find(const Engine::Name& name)", &T::find<Object>);
		r.method("Engine::Refl::ClassInfo@ refl_class_info() const", &T::refl_class_info);


		String current_type = r.class_name();
		for (auto i = info->parent; i; i = i->parent)
		{
			if (!i->is_scriptable)
			{
				continue;
			}

			// upcast
			String opconv           = Strings::format("Engine::Refl::{}@ opConv()", i->class_name.to_string());
			String const_opconv     = Strings::format("const Engine::Refl::{}@ opConv() const", i->class_name.to_string());
			String opimplconv       = Strings::format("Engine::Refl::{}@ opImplConv()", i->class_name.to_string());
			String const_opimplconv = Strings::format("const Engine::Refl::{}@ opImplConv() const", i->class_name.to_string());

			r.method(opconv.c_str(), self_address);
			r.method(const_opconv.c_str(), self_address);
			r.method(opimplconv.c_str(), self_address);
			r.method(const_opimplconv.c_str(), self_address);

			// downcast
			String opcast       = Strings::format("{}@ opCast()", current_type);
			String const_opcast = Strings::format("const {}@ opCast() const", current_type);

			auto r = ScriptClassRegistrar::existing_class(Strings::format("Engine::Refl::{}", i->class_name.to_string()));
			r.method(opcast.c_str(), downcast);
			r.method(const_opcast.c_str(), downcast);
		}
	}

	static void on_init()
	{
		ScriptClassRegistrar::RefInfo info;
		info.implicit_handle = true;
		info.no_count        = true;

		{
			ScriptEnumRegistrar r("Engine::Refl::FindFlags");
			r.set("None", FindFlags::None);
			r.set("CreateScope", FindFlags::CreateScope);
			r.set("IsRequired", FindFlags::IsRequired);
			r.set("DisableReflectionCheck", FindFlags::DisableReflectionCheck);
		}
		{
			auto r = ScriptClassRegistrar::reference_class("Engine::Refl::ClassInfo", info);
			r.property("const Name class_name", &ClassInfo::class_name);
			r.property("const ClassInfo@ parent", &ClassInfo::parent);
			r.method("bool is_a(const ClassInfo@ info) const", &ClassInfo::is_a);
		}

		auto r = ScriptClassRegistrar::reference_class("Engine::Refl::Object", info);
		r.static_function("Engine::Refl::ClassInfo@ static_refl_class_info()", &Object::static_refl_class_info);
		r.static_function("bool is_valid(Object@ object)", Object::is_valid);
		r.static_function("Object@ static_root()", Object::static_root);

		r.static_function("Object@ static_find(Engine::StringView name, int flags = 0)", Object::static_find<Object>);
		r.static_function("Object@ static_require(StringView name, int flags = 0)", Object::static_require<Object>);
		Object::register_layout(r, Object::static_refl_class_info(), script_downcast<Object>);
	}

	static ReflectionInitializeController initializer(on_init, "Engine::Refl::Object");
}// namespace Engine::Refl
