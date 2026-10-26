#include <Core/reflection/new/property.hpp>

namespace Trinex::Refl::New
{
	Property::Property(Name name, Type* type, u64 flags, Object* owner) : Object(name, flags, owner), m_type(retain_ref(type))
	{
		trinex_verify_msg(m_type && m_type->size() > 0 && m_type->alignment() > 0,
		                  "A property must have storage, so its type cannot be void");
	}

	Object* Property::resolve(const Class* base)
	{
		if (Object* object = Object::resolve(base))
		{
			return object;
		}

		return m_type->resolve(base);
	}

	const Object* Property::resolve(const Class* base) const
	{
		if (const Object* object = Object::resolve(base))
		{
			return object;
		}

		return m_type->resolve(base);
	}
} // namespace Trinex::Refl::New
