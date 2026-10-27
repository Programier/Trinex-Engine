#include <Core/object.hpp>
#include <Core/reflection/class.hpp>

namespace Trinex
{
	bool Object::is(const Refl::Class* base) const
	{
		return m_reflection->is(base);
	}
} // namespace Trinex
