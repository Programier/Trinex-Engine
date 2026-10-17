#include <Core/reflection/new/property.hpp>


namespace Trinex::Refl::New
{
	Property::Property(Name name, const Type* type, u64 flags) : Field(name, type, flags) {}
}// namespace Trinex::Refl::New
