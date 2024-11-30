#include <Core/arguments.hpp>
#include <Core/base_engine.hpp>
#include <Core/string_functions.hpp>
#include <Engine/settings.hpp>
#include <Window/config.hpp>

namespace Engine
{
	WindowConfig::WindowConfig(bool init)
	{
		if (init)
			initialize();
	}

	WindowConfig& WindowConfig::initialize()
	{
		attributes.clear();
		orientations.clear();
		attributes.insert(Settings::Window::attributes.begin(), Settings::Window::attributes.end());
		orientations.insert(Settings::Window::orientations.begin(), Settings::Window::orientations.end());
		title      = Settings::Window::title;
		client     = Settings::Window::client;
		size.x     = Settings::Window::size_x;
		size.y     = Settings::Window::size_y;
		position.x = Settings::Window::pos_x;
		position.y = Settings::Window::pos_y;
		vsync      = Settings::Window::vsync;
		return *this;
	}

	bool WindowConfig::contains_attribute(WindowAttribute attribute) const
	{
		auto it = attributes.find(attribute);
		if (it == attributes.end())
			return false;
		return true;
	}
}// namespace Engine
