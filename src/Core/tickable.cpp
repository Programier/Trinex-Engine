#include <Core/tickable.hpp>

namespace Trinex
{
	trinex_implement_registry(TickableObject);

	TickableObject& TickableObject::begin_frame()
	{
		return *this;
	}

	TickableObject& TickableObject::update(float dt)
	{
		return *this;
	}

	TickableObject& TickableObject::end_frame()
	{
		return *this;
	}

	bool TickableObject::is_tickable() const
	{
		return true;
	}
}// namespace Trinex
