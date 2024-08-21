#include <Engine/settings.hpp>
#include <Engine/splash_config.hpp>


namespace Engine
{

	SplashConfig::SplashConfig()
	{
		init();
	}

	SplashConfig& SplashConfig::init()
	{
		image_path          = Settings::e_splash_image;
		font_path           = Settings::e_splash_font;
		startup_text_size   = Settings::e_splash_startup_text_size;
		version_text_size   = Settings::e_splash_version_text_size;
		copyright_text_size = Settings::e_splash_copyright_text_size;
		game_name_text_size = Settings::e_splash_game_name_text_size;
		return *this;
	}
}// namespace Engine
