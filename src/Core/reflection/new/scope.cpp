#include <Core/reflection/new/scope.hpp>

namespace Trinex::Refl::New
{
	Object* Object::root()
	{
		return Scope::root();
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

	Scope* Scope::root()
	{
		static ExternalLifetime<Scope> scope = ExternalLifetime<Scope>("Reflection");
		return &scope;
	}

	Scope* Scope::create(InitializerList<Name> path)
	{
		Scope* scope = root();

		if (path.size() == 0)
			return scope;

		for (Name entry : path)
		{
			if (!entry.is_valid())
				return nullptr;

			if (Object* object = scope->find(entry))
			{
				if (object->kind() != ObjectKind::Scope)
					return nullptr;

				scope = static_cast<Scope*>(object);
				continue;
			}

			Ref<Scope> child = Ref<Scope>::make(entry, Object::None, scope);

			if (child->owner() != scope)
				return nullptr;

			scope = child.value();
		}

		return scope;
	}

	ObjectKind Scope::kind() const
	{
		return ObjectKind::Scope;
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
