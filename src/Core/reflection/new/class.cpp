#include <Core/reflection/new/class.hpp>

namespace Trinex::Refl::New
{
	Class::Class(Name name, u32 size, u32 alignment, u64 flags, const Class* base, const VTable* vtable, Object* owner)
	    : Struct(name, size, alignment, flags, base, vtable, owner)
	{}
}// namespace Trinex::Refl::New
