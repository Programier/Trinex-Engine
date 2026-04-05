#include <Core/tickable.hpp>

namespace Trinex
{
	trinex_implement_registry(TickableObject);
	
	TickableObject& TickableObject::update(float dt)
	{
		return *this;
	}

	bool TickableObject::is_tickable() const
	{
		return true;
	}

	bool TickableObject::is_tickable_when_paused() const
	{
		return true;
	}
}// namespace Trinex
