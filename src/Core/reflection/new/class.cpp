#include <Core/reflection/new/class.hpp>

namespace Trinex::Refl::New
{
	Class::Class(Name name, u32 size, u32 alignment, const Class* base, u64 flags, const VTable* vtable)
		: Struct(name, size, alignment, base, flags, vtable)
	{}

	ObjectKind Class::kind() const
	{
		return ObjectKind::Class;
	}
}// namespace Trinex::Refl::New
