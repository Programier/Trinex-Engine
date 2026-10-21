#include <Core/reflection/new/pointer_type.hpp>

namespace Trinex::Refl::New
{
	PointerType::PointerType(Name name, Type* type, Object* owner)
	    : Type(name, sizeof(void*), alignof(void*), 0), m_type(retain_ref(type))
	{}
}// namespace Trinex::Refl::New
