#include <Core/constants.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/exception.hpp>
#include <Core/reflection/object.hpp>
#include <Core/string_functions.hpp>

namespace Engine::Refl
{
	static Set<Object*> m_instances;
	static bool m_check_exiting_instance           = true;
	static thread_local StringView m_accepted_name = "";
	static Map<StringView, Object*> m_type_name_map;

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

	Object::Link::Link(const char* name, const Link* const parent) : class_name(Strings::class_name_sv_of(name)), parent(parent)
	{}

	bool Object::Link::is_a(const Link* const link) const
	{
		const Link* self = this;
		while (self && self != link) self = self->parent;
		return self != nullptr;
	}

	const Object::Link* Object::static_link()
	{
		static const Link link("Type", nullptr);

		return &link;
	}

	String Object::concat_scoped_name(StringView scope, StringView name)
	{
		if (scope == static_root()->m_name)
			return String(name);
		return Strings::concat_scoped_name(scope, name);
	}

	void Object::accept_next_object(StringView name)
	{
		if (name != "Root")
		{
			trinex_always_check(static_find(name, FindFlags::DisableReflectionCheck) == nullptr,
								"Object with same name already exist");
		}

		trinex_always_check(!name.empty(), "Reflection object name cannot be empty!");
		m_accepted_name = name;
	}

	Object::Object() : m_owner(nullptr), m_name(Strings::class_name_of(m_accepted_name))
	{
		trinex_always_check(!m_accepted_name.empty(), "Reflection object name cannot be empty!");
		trinex_always_check(m_name.is_valid(), "Reflection object name cannot be empty!");
		m_name_splitted = Strings::make_sentence(m_name);

		auto owner_name = Strings::namespace_sv_of(m_accepted_name);

		if (owner_name.empty())
		{
			if (m_name != "Root")
			{
				owner(static_root());
			}
		}
		else
		{
			owner(static_find(owner_name, FindFlags::CreateScope));
		}

		m_instances.insert(this);
		m_accepted_name = "";
	}

	Object::~Object()
	{
		m_instances.erase(this);
	}

	const Object::Link* Object::link() const
	{
		return This::static_link();
	}

	const Name& Object::class_name() const
	{
		return link()->class_name;
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

	const String& Object::name_splitted() const
	{
		return m_name_splitted;
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

	String Object::full_name() const
	{
		String result;
		result.reserve(64);
		full_name(result);
		return result;
	}

	String Object::scope_name() const
	{
		if (m_owner)
		{
			return m_owner->full_name();
		}

		return "";
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

			object->on_destroy(object);
			delete object;

			m_check_exiting_instance = check_exiting_instance_tmp;
			return true;
		}
		return false;
	}
}// namespace Engine::Refl
