#include <Core/group.hpp>
#include <Core/property.hpp>
#include <Core/reflection/struct.hpp>

namespace Engine::Refl
{
	implement_reflect_type(Struct);

	Struct::Struct(Struct* parent, BitMask flags, StringView type_name) : flags(flags), m_parent(parent), m_type_name(type_name)
	{
		if (parent)
		{
			parent->m_childs.insert(this);
		}

		if (!m_type_name.empty())
			bind_type_name(m_type_name);
	}

	void Struct::destroy_childs()
	{
		while (!m_childs.empty())
		{
			Struct* child_struct = *m_childs.begin();
			delete child_struct;
		}
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

	Struct* Struct::parent() const
	{
		return m_parent;
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
			result.emplace_back(current->full_name());
			current = current->parent();
			--level;
		}

		return result;
	}

	const Set<Struct*>& Struct::childs() const
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

	Group* Struct::group() const
	{
		return m_group;
	}

	Struct::~Struct()
	{
		for (auto* prop : m_properties)
		{
			delete prop;
		}

		m_properties.clear();

		destroy_childs();

		if (m_parent)
		{
			m_parent->m_childs.erase(this);
		}

		if (!m_type_name.empty())
			unbind_type_name(m_type_name);
	}
}// namespace Engine::Refl
