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


	Struct::Struct(const Name& ns, const Name& name, const Name& _parent)
	    : m_alloc(nullptr), m_free(nullptr), m_full_name(Strings::concat_scoped_name(ns, name)), m_namespace_name(ns),
		  m_base_name(name), m_parent(_parent)
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

	Struct::Struct(const Name& ns, const Name& name, Struct* parent) : Struct(ns, name)
	{
		m_parent_struct = parent;
		if (m_parent_struct)
		{
			m_parent = m_parent_struct->full_name();
			m_parent_struct->m_childs.insert(this);
		}
	}

	void Struct::destroy_childs()
	{
		while (!m_childs.empty())
		{
			Struct* child_struct = *m_childs.begin();
			delete child_struct;
		}
	}

	ENGINE_EXPORT bool Struct::create_internal(const Name& ns, const Name& name, Struct* parent, Struct*& self)
	{
		self = static_find(Strings::concat_scoped_name(ns, name));

		if (!self)
		{
			self = new Struct(ns, name, parent);
			return true;
		}

		return false;
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

	const String& Struct::name_splitted() const
	{
		return m_base_name_splitted;
	}

	const Name& Struct::full_name() const
	{
		return m_full_name;
	}

	const Name& Struct::namespace_name() const
	{
		return m_namespace_name;
	}

	const Name& Struct::name() const
	{
		return m_base_name;
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
		return m_alloc();
	}

	const Struct& Struct::destroy_struct(void* obj) const
	{
		m_free(obj);
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

	const TreeSet<Struct*, Struct::StructCompare>& Struct::child_structs() const
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

		destroy_childs();

		if (m_parent_struct)
		{
			m_parent_struct->m_childs.erase(this);
		}

		if (m_group)
		{
			m_group->remove_struct(this);
		}

		internal_struct_map().erase(Strings::hash_of(m_full_name));
	}
}// namespace Engine
