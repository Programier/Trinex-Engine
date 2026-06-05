#pragma once

#include <Core/etl/map.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/math/vector.hpp>

namespace Trinex
{
	class Window;

	class ENGINE_EXPORT TouchScreenSystem : public Singletone<TouchScreenSystem, EmptySingletoneParent>
	{
	public:
		static TouchScreenSystem* s_instance;

	private:
		friend class Singletone<TouchScreenSystem, EmptySingletoneParent>;

		Map<usize, Vector2u> m_finger_locations;
		TouchScreenSystem() = default;

	public:
		Vector2u finger_location(usize index, Window* window = nullptr) const;
		void update_finger(usize index, Vector2u value);
	};
}// namespace Trinex
