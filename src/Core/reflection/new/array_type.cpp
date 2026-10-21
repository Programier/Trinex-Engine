#include <Core/etl/array.hpp>
#include <Core/reflection/new/array_type.hpp>


namespace Trinex::Refl::New
{
	using VTable = ArrayType::VTable;

	VTable::VTable(Type* argument) : m_argument(retain_ref(argument))
	{
		trinex_verify(argument);
	}

	ArrayType::ArrayType(Name name, Type* argument, Object* owner)
	    : ContainerType(name, sizeof(Array<i32>), alignof(Array<i32>), 0, &m_vtable, owner), m_vtable(argument)
	{}

	Type* ArrayType::argument(usize idx) const
	{
		if (idx == 0)
		{
			return argument();
		}

		return nullptr;
	}
}// namespace Trinex::Refl::New
