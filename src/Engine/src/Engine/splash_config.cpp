#include <Core/config_manager.hpp>
#include <Engine/splash_config.hpp>


namespace Engine
{

    SplashConfig::SplashConfig()
    {
        init();
    }

    SplashConfig& SplashConfig::init()
    {
        image_path          = ConfigManager::get_path("Engine::Splash::image");
        font_path           = ConfigManager::get_path("Engine::Splash::font_file");
        startup_text_size   = ConfigManager::get_int("Engine::Splash::startup_text_size");
        version_text_size   = ConfigManager::get_int("Engine::Splash::version_text_size");
        copyright_text_size = ConfigManager::get_int("Engine::Splash::copyright_text_size");
        game_name_text_size = ConfigManager::get_int("Engine::Splash::game_name_text_size");
        return *this;
    }
}// namespace Engine
