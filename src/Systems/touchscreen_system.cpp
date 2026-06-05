#include <Systems/touchscreen_system.hpp>

namespace Trinex
{
	TouchScreenSystem* TouchScreenSystem::s_instance = nullptr;

	Vector2u TouchScreenSystem::finger_location(usize index, Window*) const
	{
		if (auto found = m_finger_locations.find(index); found != m_finger_locations.end())
		{
			return found->second;
		}

		return {};
	}

	void TouchScreenSystem::update_finger(usize index, Vector2u value)
	{
		m_finger_locations[index] = value;
	}
}// namespace Trinex
