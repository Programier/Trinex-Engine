#include <Core/etl/array.hpp>
#include <Core/reflection/new/array_type.hpp>


namespace Trinex::Refl::New
{
	using VTable = ArrayType::VTable;

	VTable::VTable(Type* argument) : m_argument(retain_ref(argument))
	{
		trinex_verify(argument);
	}

	void VTable::construct(void* dst, usize count) const {}

	void VTable::destruct(void* value, usize count) const {}

	void VTable::copy_construct(void* dst, const void* src, usize count) const {}

	void VTable::move_construct(void* dst, void* src, usize count) const {}

	void VTable::copy_assign(void* dst, const void* src, usize count) const {}

	void VTable::move_assign(void* dst, void* src, usize count) const {}

	void VTable::swap(void* lhs, void* rhs, usize count) const {}

	void VTable::relocate(void* dst, void* src, usize count) const {}

	bool VTable::equals(const void* lhs, const void* rhs) const
	{
		return false;
	}

	i32 VTable::compare(const void* lhs, const void* rhs) const
	{
		return false;
	}

	usize VTable::size(const void* container) const
	{
		return 0;
	}

	void VTable::clear(void* container) const {}

	ArrayType::ArrayType(Name name, Type* argument, Object* owner)
	    : ContainerType(name, sizeof(Array<i32>), alignof(Array<i32>)), m_vtable(argument)
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
