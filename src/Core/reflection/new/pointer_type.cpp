#include <Core/reflection/new/native.hpp>
#include <Core/reflection/new/of.hpp>
#include <Core/reflection/new/pointer_type.hpp>

namespace Trinex::Refl::New
{
	PointerType::PointerType(Name name, Type* type, Object* owner)
	    : Type(name, sizeof(void*), alignof(void*), 0, vtable(), owner), m_type(retain_ref(type))
	{}

	PointerType* PointerType::from(Type* type)
	{
		if (type == nullptr)
			return nullptr;

		Object* owner = type->owner().value();

		if (owner == nullptr)
			return nullptr;

		Name name = type->name().to_string() + "*";

		if (Object* type = owner->find(name))
		{
			return static_cast<PointerType*>(type);
		}

		Ref<PointerType> pointer = Ref<PointerType>::make(name, type, owner);
		return pointer.value();
	}

	const PointerType::VTable* PointerType::vtable()
	{
		static Native::TypeVTable<void*, VTable> vtable;
		return &vtable;
	}

	Object* PointerType::resolve(const Class* base)
	{
		if (Object* object = Type::resolve(base))
		{
			return object;
		}

		return m_type->resolve(base);
	}

	const Object* PointerType::resolve(const Class* base) const
	{
		if (const Object* object = Type::resolve(base))
		{
			return object;
		}

		return m_type->resolve(base);
	}

	trinex_api Type* pointer_type_of(Type* type)
	{
		return PointerType::from(type);
	}
} // namespace Trinex::Refl::New
