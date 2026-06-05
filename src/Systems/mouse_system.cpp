#include <Platform/platform.hpp>
#include <Systems/mouse_system.hpp>
#include <Window/window.hpp>

namespace Trinex
{
	MouseSystem* MouseSystem::s_instance = nullptr;

	MouseSystem& MouseSystem::relative_mode(bool flag, Window* window)
	{
		if (window)
		{
			m_relative_mode[window] = flag;
		}

		Platform::WindowManager::mouse_relative_mode(flag);
		return *this;
	}

	bool MouseSystem::is_relative_mode(Window* window) const
	{
		if (window == nullptr)
			return false;

		if (auto found = m_relative_mode.find(window); found != m_relative_mode.end())
		{
			return found->second;
		}

		return false;
	}
}// namespace Trinex
