#pragma once

#include <Core/etl/map.hpp>
#include <Core/etl/singletone.hpp>

namespace Trinex
{
	class Window;

	class ENGINE_EXPORT MouseSystem : public Singletone<MouseSystem, EmptySingletoneParent>
	{
	public:
		static MouseSystem* s_instance;

	private:
		friend class Singletone<MouseSystem, EmptySingletoneParent>;

		Map<Window*, bool> m_relative_mode;
		MouseSystem() = default;

	public:
		MouseSystem& relative_mode(bool flag, Window* window);
		bool is_relative_mode(Window* window) const;
	};
}// namespace Trinex
