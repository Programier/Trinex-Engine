#include <Core/group.hpp>
#include <Core/property.hpp>
#include <Core/reflection/struct.hpp>

namespace Engine::Refl
{
	implement_reflect_type(Struct);

	Struct::Struct(Struct* parent, BitMask flags) : flags(flags), m_parent(parent)
	{
		if (parent)
		{
			parent->m_childs.insert(this);
		}
	}

	Struct& Struct::construct()
	{
		Super::construct();

		if (is_scriptable() && is_native())
		{
			register_scriptable_instance();
		}
		return *this;
	}

	Struct& Struct::register_scriptable_instance()
	{
		return *this;
	}

	Struct& Struct::initialize()
	{
		Super::initialize();
		if (m_parent && !m_parent->is_initialized())
		{
			static_initialize(m_parent);
		}
		return *this;
	}

	void Struct::destroy_childs()
	{
		while (!m_childs.empty())
		{
			Struct* child_struct = *m_childs.begin();
			delete child_struct;
		}
	}

	void* Struct::create_struct()
	{
		throw EngineException("Unimplemented method");
	}

	Struct& Struct::destroy_struct(void* obj)
	{
		throw EngineException("Unimplemented method");
		return *this;
	}

	StringView Struct::type_name() const
	{
		throw EngineException("Unimplemented method");
		return "";
	}

	size_t Struct::size() const
	{
		throw EngineException("Unimplemented method");
		return 0;
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

	bool Struct::is_asset() const
	{
		return flags(Struct::IsAsset);
	}

	bool Struct::is_native() const
	{
		return flags(IsNative);
	}

	bool Struct::is_scriptable() const
	{
		return flags(IsScriptable);
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

	Struct& Struct::add_property(Engine::Property* prop)
	{
		m_properties.push_back(prop);
		return *this;
	}

	const Vector<class Engine::Property*>& Struct::properties() const
	{
		return m_properties;
	}

	static FORCE_INLINE Engine::Property* find_prop_internal(Struct* self, const Name& name)
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

	class Engine::Property* Struct::find_property(const Name& name, bool recursive)
	{
		if (recursive)
		{
			Struct* self           = this;
			Engine::Property* prop = nullptr;

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

	bool Struct::archive_process(void* object, Archive& ar)
	{
		return true;
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
	}
}// namespace Engine::Refl
