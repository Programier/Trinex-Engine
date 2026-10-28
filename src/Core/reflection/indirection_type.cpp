#include <Core/reflection/indirection_type.hpp>
#include <Core/reflection/native.hpp>
#include <Core/reflection/of.hpp>

namespace Trinex::Refl
{
	IndirectionType::IndirectionType(Name name, Type* type, Object* owner)
	    : Type(name, sizeof(void*), alignof(void*), 0, vtable()), m_type(retain_ref(type))
	{
		if (owner)
		{
			Object::owner(owner);
		}
	}

	IndirectionType* IndirectionType::from(Type* type, bool rvalue)
	{
		if (type == nullptr)
			return nullptr;

		Object* owner = type->owner().value();

		if (owner == nullptr)
			return nullptr;

		Name name = type->name().to_string() + (rvalue ? "*" : "&&");

		if (Object* type = owner->find(name))
		{
			return static_cast<IndirectionType*>(type);
		}

		Ref<IndirectionType> pointer = Ref<IndirectionType>::make(name, type, owner);
		return pointer.value();
	}

	const IndirectionType::VTable* IndirectionType::vtable()
	{
		static Native::TypeVTable<void*, VTable> vtable;
		return &vtable;
	}

	Object* IndirectionType::resolve(const Class* base)
	{
		if (Object* object = Type::resolve(base))
		{
			return object;
		}

		return m_type->resolve(base);
	}

	const Object* IndirectionType::resolve(const Class* base) const
	{
		if (const Object* object = Type::resolve(base))
		{
			return object;
		}

		return m_type->resolve(base);
	}

	trinex_api Type* indirection_type_of(Type* type, bool rvalue)
	{
		return IndirectionType::from(type);
	}
} // namespace Trinex::Refl
