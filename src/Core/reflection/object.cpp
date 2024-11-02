#include <Core/constants.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/reflection/object.hpp>
#include <Core/string_functions.hpp>

namespace Engine::Refl
{
	namespace Meta
	{
		ENGINE_EXPORT Name display_name = "display_name";
		ENGINE_EXPORT Name tooltip      = "tooltip";
		ENGINE_EXPORT Name description  = "description";
	}// namespace Meta

	static Set<Object*> m_instances;
	static Map<StringView, Object*> m_type_name_map;

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

	Object::ReflClassInfo::ReflClassInfo(const char* name, const ReflClassInfo* const parent)
		: class_name(Strings::class_name_sv_of(name)), parent(parent)
	{}

	bool Object::ReflClassInfo::is_a(const ReflClassInfo* const info) const
	{
		const ReflClassInfo* self = this;
		while (self && self != info) self = self->parent;
		return self != nullptr;
	}

	const Object::ReflClassInfo* Object::static_refl_class_info()
	{
		static const ReflClassInfo info("Object", nullptr);
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

	Object::Object() : m_owner(nullptr), m_name(m_next_object_name)
	{
		trinex_always_check(m_has_next_object_info, "Use new_instance or new_child method for creating reflection objects!");

		if (m_next_object_owner)
		{
			owner(m_next_object_owner);
		}

		m_instances.insert(this);
		m_next_object_name     = "";
		m_next_object_owner    = nullptr;
		m_has_next_object_info = false;
	}

	Object::~Object()
	{
		m_instances.erase(this);

		if (m_metadata)
		{
			delete m_metadata;
		}
	}

	const Object::ReflClassInfo* Object::refl_class_info() const
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

	const String& Object::display_name() const
	{
		if (auto* meta = find_metadata(Meta::display_name))
			return *meta;

		return m_name.to_string();
	}

	const String& Object::tooltip() const
	{
		return metadata(Meta::tooltip);
	}

	const String& Object::description() const
	{
		if (auto* meta = find_metadata(Meta::description))
			return *meta;

		return m_name.to_string();
	}

	Object& Object::display_name(StringView name)
	{
		return metadata(Meta::display_name, name);
	}

	Object& Object::tooltip(StringView text)
	{
		return metadata(Meta::tooltip, text);
	}

	Object& Object::description(StringView text)
	{
		return metadata(Meta::description, text);
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

	void Object::bind_type_name(StringView name)
	{
		trinex_always_check(static_find_by_type_name(name) == nullptr, "Type id currently in use");
		m_type_name_map[name] = this;
	}

	void Object::unbind_type_name(StringView name)
	{
		m_type_name_map.erase(name);
	}

	const String* Object::find_metadata(const Name& name) const
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

	const String& Object::metadata(const Name& name) const
	{
		if (auto meta = find_metadata(name))
		{
			return *meta;
		}
		return default_value_of<String>();
	}

	Object& Object::metadata(const Name& name, StringView meta)
	{
		if (m_metadata == nullptr)
		{
			m_metadata = new MetaData();
		}

		(*m_metadata)[name] = String(meta);
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

	Object* Object::static_find_by_type_name(StringView name)
	{
		auto it = m_type_name_map.find(name);
		if (it == m_type_name_map.end())
			return nullptr;
		return it->second;
	}

	bool Object::destroy_instance(Object* object)
	{
		if (!m_check_exiting_instance || m_instances.find(object) != m_instances.end())
		{
			bool check_exiting_instance_tmp = m_check_exiting_instance;
			m_check_exiting_instance        = true;

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
}// namespace Engine::Refl
