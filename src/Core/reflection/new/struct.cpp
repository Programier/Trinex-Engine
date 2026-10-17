#include <Core/reflection/new/function.hpp>
#include <Core/reflection/new/property.hpp>
#include <Core/reflection/new/struct.hpp>

namespace Trinex::Refl::New
{
	Struct::Struct(Name name, u32 size, u32 alignment, const Struct* base, u64 flags, const VTable* vtable)
	    : Type(name, size, alignment, flags, vtable), m_base(retain_ref(base))
	{}

	bool Struct::register_child(Object* object)
	{
		if (object == nullptr || object == this || m_members.contains(object->name()))
			return false;

		for (Object* scope = this; scope != nullptr;)
		{
			if (scope == object)
				return false;

			Ref<Object> parent = scope->owner();
			scope              = parent.value();
		}

		m_members.insert(retain_ref(object));
		return true;
	}

	bool Struct::unregister_child(Object* object)
	{
		if (object == nullptr)
			return false;

		if (auto it = m_members.find(object->name()); it != m_members.end())
		{
			m_members.erase(it);
			return true;
		}

		return false;
	}

	ObjectKind Struct::kind() const
	{
		return ObjectKind::Struct;
	}

	Object* Struct::find(Name name) const
	{
		if (auto it = m_members.find(name); it != m_members.end())
		{
			return it->value();
		}

		return nullptr;
	}

	bool Struct::is_a(const Struct* other) const
	{
		for (const Struct* current = this; current != nullptr; current = current->base())
		{
			if (current == other)
				return true;
		}
		return false;
	}
}// namespace Trinex::Refl::New
