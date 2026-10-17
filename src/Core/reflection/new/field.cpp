#include <Core/reflection/new/field.hpp>
#include <Core/reflection/new/type.hpp>

namespace Trinex::Refl::New
{
	Field::Field(Name name, const Type* type, u64 flags) : Object(name, flags), m_type(type) {}
}// namespace Trinex::Refl::New
