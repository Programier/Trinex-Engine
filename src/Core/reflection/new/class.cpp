#include <Core/reflection/new/class.hpp>

namespace Trinex::Refl::New
{
	Class::Class(Name name, u32 size, u32 alignment, u64 flags, Array<Ref<Struct>> bases, const VTable* vtable, Object* owner)
	    : Struct(name, size, alignment, flags, etl::move(bases), vtable, owner)
	{}
}// namespace Trinex::Refl::New
