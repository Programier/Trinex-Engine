#include <Core/reflection/new/class.hpp>
#include <Core/reflection/new/object.hpp>

namespace Trinex::Refl::New
{
	Object::Object(Name name, u64 flags, Object* owner) : m_flags(flags), m_name(name)
	{
		m_owner = owner && owner->register_child(this) ? owner : nullptr;
	}

	Class* Object::reflection() const
	{
		return class_of<Object>();
	}

	Object* Object::find(Name name) const
	{
		return nullptr;
	}

	bool Object::owner(Object* object)
	{
		if (m_owner == object)
			return true;
		if (object == this)
			return false;

		Ref<Object> keep_alive = retain_ref(this);

		if (auto lock = m_owner.lock())
		{
			if (!lock->unregister_child(this))
			{
				return false;
			}
		}

		if (object == nullptr)
			return true;

		if (!object->register_child(this))
			return false;

		m_owner = object;
		return true;
	}

	bool Object::is(const Class* base) const
	{
		if (Class* instance = reflection())
			return base && instance->is(base);

		return false;
	}

	bool Object::register_child(Object* object)
	{
		return false;
	}

	bool Object::unregister_child(Object* object)
	{
		return false;
	}
}// namespace Trinex::Refl::New
