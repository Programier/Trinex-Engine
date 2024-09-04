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
		attributes.insert(Settings::w_attributes.begin(), Settings::w_attributes.end());
		orientations.insert(Settings::w_orientations.begin(), Settings::w_orientations.end());
		title      = Settings::w_title;
		client     = Settings::w_client;
		size.x     = Settings::w_size_x;
		size.y     = Settings::w_size_y;
		position.x = Settings::w_pos_x;
		position.y = Settings::w_pos_y;
		vsync      = Settings::w_vsync;
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
