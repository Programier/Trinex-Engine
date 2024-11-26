#include <Core/config_manager.hpp>
#include <Core/editor_config.hpp>
#include <Core/engine_loading_controllers.hpp>

namespace Engine::Settings
{
	String ed_font_path = "resources/TrinexEditor/fonts/Source Code Pro/SourceCodePro-Bold.ttf";
	float ed_font_size  = 18.f;
	bool ed_show_grid   = true;

	static PreInitializeController initialize([]() {
		ConfigManager::register_property("ed_font_path", ed_font_path, "editor");
		ConfigManager::register_property("ed_font_size", ed_font_size, "editor");
		ConfigManager::register_property("ed_show_grid", ed_show_grid, "editor");
	});
}// namespace Engine::Settings
