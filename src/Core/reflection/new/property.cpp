#include <Core/reflection/new/property.hpp>


namespace Trinex::Refl::New
{
	Property::Property(Name name, Type* type, u64 flags, Object* owner) : Object(name, flags, owner), m_type(retain_ref(type)) {}
}// namespace Trinex::Refl::New
