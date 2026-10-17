#include <Core/reflection/new/scope.hpp>

namespace Trinex::Refl::New
{
	Scope* Object::root()
	{
		static ExternalLifetime<Scope> scope = ExternalLifetime<Scope>("Reflection");
		return &scope;
	}

	bool Scope::register_child(Object* object)
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

	bool Scope::unregister_child(Object* object)
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

	Object* Scope::find(Name name) const
	{
		if (auto it = m_members.find(name); it != m_members.end())
		{
			return it->value();
		}

		return nullptr;
	}
}// namespace Trinex::Refl::New
