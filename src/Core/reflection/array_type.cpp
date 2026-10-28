#include <Core/etl/array.hpp>
#include <Core/reflection/array_type.hpp>
#include <Core/reflection/of.hpp>
#include <Core/reflection/scope.hpp>

namespace Trinex::Refl
{
	using VTable = ArrayType::VTable;

	VTable::VTable(Type* argument) : m_argument(retain_ref(argument))
	{
		trinex_verify(argument);
	}

	ArrayType::ArrayType(Name name, Type* argument, Object* owner)
	    : ContainerType(name, sizeof(Array<i32>), alignof(Array<i32>), 0, &m_vtable), m_vtable(argument)
	{
		if (owner)
		{
			Object::owner(owner);
		}
	}

	ArrayType* ArrayType::from(Type* type)
	{
		if (type == nullptr)
			return nullptr;

		Scope* scope = Scope::trinex();

		const Name name = "Array<" + type->name().to_string() + ">";

		if (Object* object = scope->find(name))
		{
			return static_cast<ArrayType*>(object);
		}

		return Ref<ArrayType>::make(name, type, Scope::trinex()).value();
	}

	Type* ArrayType::argument(usize idx) const
	{
		if (idx == 0)
		{
			return argument();
		}

		return nullptr;
	}

	trinex_api Type* array_type_of(Type* type)
	{
		return ArrayType::from(type);
	}
} // namespace Trinex::Refl
