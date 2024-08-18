#include <Core/config_manager.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Engine/settings.hpp>


static void load_configs()
{
	Engine::Settings::e_splash_font = "resources/fonts/Source Code Pro/SourceCodePro-Bold.ttf";
	Engine::ConfigManager::load_config_from_file("editor.config");
}

static Engine::ConfigsInitializeController configs_initializer(load_configs, "EditorConfig");
