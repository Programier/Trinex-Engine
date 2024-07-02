#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/config_manager.hpp>
#include <Core/engine.hpp>
#include <Core/package.hpp>
#include <Engine/engine_start.hpp>
#include <Engine/settings.hpp>
#include <Window/config.hpp>
#include <editor_config.hpp>


static void load_configs()
{
    // Engine::ConfigManager::load_from_file("editor.config");
    // Engine::editor_config.update();
}

static Engine::ConfigsPreInitializeController preinitialize_controller([]() {
    Engine::Settings::e_splash_font = "resources/fonts/Source Code Pro/SourceCodePro-Bold.ttf";
});

static Engine::ConfigsInitializeController configs_initializer(load_configs, "EditorConfig", {"EngineConfig"});
