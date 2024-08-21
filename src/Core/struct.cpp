#include <Core/exception.hpp>
#include <Core/group.hpp>
#include <Core/property.hpp>
#include <Core/string_functions.hpp>
#include <Core/struct.hpp>

namespace Engine
{
	static StructMap& internal_struct_map()
	{
		static StructMap map;
		return map;
	}

	static void on_destroy()
	{
		auto& map = internal_struct_map();

		while (!map.empty())
		{
			Struct* struct_instance = map.begin()->second;
			delete struct_instance;
		}
		internal_struct_map().clear();
	}

	static PostDestroyController destroy_struct_map(on_destroy);


	Struct::Struct(const Name& name, const Name& _parent)
	    : m_struct_constructor(nullptr), m_full_name(name), m_namespace_name(Strings::namespace_sv_of(name)),
	      m_base_name(Strings::class_name_sv_of(name)), m_parent(_parent)
	{
		m_base_name_splitted = Strings::make_sentence(m_base_name.to_string());

		if (m_parent.is_valid())
		{
			parent();
		}

		auto& it = internal_struct_map()[Strings::hash_of(m_full_name)];
		if (it)
		{
			delete it;
		}
		it = this;
	}

	Struct::Struct(const Name& name, Struct* parent) : Struct(name)
	{
		m_parent_struct = parent;
		if (m_parent_struct)
		{
			m_parent = m_parent_struct->name();
			m_parent_struct->m_childs.insert(this);
		}
	}

	ENGINE_EXPORT Struct* Struct::create(const Name& name, const Name& parent)
	{
		Struct* self = static_find(name);

		if (!self)
		{
			self = new Struct(name, parent);
		}

		return self;
	}

	ENGINE_EXPORT Struct* Struct::static_find(const StringView& name, bool requred)
	{
		auto& map = internal_struct_map();
		auto it   = map.find(Strings::hash_of(name));
		if (it == map.end())
		{
			// Maybe initializer is not executed?
			ReflectionInitializeController().require(String(name));
			it = map.find(Strings::hash_of(name));

			if (it != map.end())
				return it->second;

			if (requred)
			{
				throw EngineException(Strings::format("Failed to find struct '{}'", name.data()));
			}
			return nullptr;
		}
		return it->second;
	}

	const String& Struct::base_name_splitted() const
	{
		return m_base_name_splitted;
	}

	const Name& Struct::name() const
	{
		return m_full_name;
	}

	const Name& Struct::namespace_name() const
	{
		return m_namespace_name;
	}

	const Name& Struct::base_name() const
	{
		return m_base_name;
	}

	const Name& Struct::parent_name() const
	{
		return m_parent;
	}

	Struct* Struct::parent() const
	{
		if (m_parent_struct == nullptr)
		{
			m_parent_struct = static_find(m_parent);
			if (m_parent_struct)
			{
				m_parent_struct->m_childs.insert(const_cast<Struct*>(this));
			}
		}
		return m_parent_struct;
	}

	void* Struct::create_struct() const
	{
		if (m_struct_constructor)
		{
			return m_struct_constructor();
		}

		return nullptr;
	}

	Struct& Struct::struct_constructor(void* (*constructor)())
	{
		m_struct_constructor = constructor;
		return *this;
	}

	Struct& Struct::group(class Group* group)
	{
		if (m_group)
		{
			m_group->remove_struct(this);
		}

		m_group = group;

		if (m_group)
		{
			m_group->add_struct(this);
		}
		return *this;
	}

	class Group* Struct::group() const
	{
		return m_group;
	}

	size_t Struct::abstraction_level() const
	{
		size_t level = 1;

		Struct* next = parent();
		while (next)
		{
			++level;
			next = next->parent();
		}

		return level;
	}

	Vector<Name> Struct::hierarchy(size_t offset) const
	{
		size_t level = abstraction_level();
		if (offset >= level)
			return {};

		level -= offset;

		Vector<Name> result;
		result.reserve(level);

		const Struct* current = this;
		while (current && level != 0)
		{
			result.emplace_back(current->m_full_name);
			current = current->parent();
			--level;
		}

		return result;
	}

	const Set<Struct*>& Struct::child_structs() const
	{
		return m_childs;
	}

	bool Struct::is_a(const Struct* other) const
	{
		const Struct* current = this;
		while (current && current != other)
		{
			current = current->parent();
		}
		return current != nullptr;
	}

	bool Struct::is_class() const
	{
		return false;
	}

	Struct& Struct::add_property(Property* prop)
	{
		m_properties.push_back(prop);
		m_grouped_properties[prop->group()].push_back(prop);
		return *this;
	}

	const Vector<class Property*>& Struct::properties() const
	{
		return m_properties;
	}


	static FORCE_INLINE Property* find_prop_internal(Struct* self, const Name& name)
	{
		for (auto& prop : self->properties())
		{
			if (prop->name() == name)
			{
				return prop;
			}
		}

		return nullptr;
	}

	class Property* Struct::find_property(const Name& name, bool recursive)
	{
		if (recursive)
		{
			Struct* self   = this;
			Property* prop = nullptr;

			while (self && (prop = find_prop_internal(self, name)) == nullptr)
			{
				self = self->parent();
			}

			return prop;
		}
		else
		{
			return find_prop_internal(this, name);
		}
		return nullptr;
	}

	const Struct::GroupedPropertiesMap& Struct::grouped_properties() const
	{
		return m_grouped_properties;
	}

	const StructMap& Struct::struct_map()
	{
		return internal_struct_map();
	}

	Struct::~Struct()
	{
		for (auto* prop : m_properties)
		{
			delete prop;
		}

		m_properties.clear();

		while (!m_childs.empty())
		{
			Struct* child_struct = *m_childs.begin();
			delete child_struct;
		}

		if (m_parent_struct)
		{
			m_parent_struct->m_childs.erase(this);
		}

		internal_struct_map().erase(Strings::hash_of(m_full_name));
	}
}// namespace Engine
