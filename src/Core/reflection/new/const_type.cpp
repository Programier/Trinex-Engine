#include <Core/reflection/new/const_type.hpp>
#include <Core/reflection/new/native.hpp>
#include <Core/reflection/new/of.hpp>

namespace Trinex::Refl::New
{
	ConstType::ConstType(Name name, Type* type, Object* owner)
	    : Type(name, sizeof(void*), alignof(void*), 0, type ? type->vtable() : nullptr, owner), m_type(retain_ref(type))
	{}

	ConstType* ConstType::from(Type* type)
	{
		if (type == nullptr)
			return nullptr;

		Object* owner = type->owner().value();

		if (owner == nullptr)
			return nullptr;

		Name name = "const " + type->name().to_string();

		if (Object* type = owner->find(name))
		{
			return static_cast<ConstType*>(type);
		}

		Ref<ConstType> pointer = Ref<ConstType>::make(name, type, owner);
		return pointer.value();
	}

	Object* ConstType::find(Name name, const Class* base) const
	{
		return m_type->find(name, base);
	}

	Object* ConstType::resolve(const Class* base)
	{
		if (Object* object = Type::resolve(base))
		{
			return object;
		}

		return m_type->resolve(base);
	}

	const Object* ConstType::resolve(const Class* base) const
	{
		if (const Object* object = Type::resolve(base))
		{
			return object;
		}

		return m_type->resolve(base);
	}

	trinex_api Type* const_type_of(Type* type)
	{
		return ConstType::from(type);
	}
} // namespace Trinex::Refl::New
