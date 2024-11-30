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
		image_path          = Settings::Splash::image;
		font_path           = Settings::Splash::font;
		startup_text_size   = Settings::Splash::startup_text_size;
		version_text_size   = Settings::Splash::version_text_size;
		copyright_text_size = Settings::Splash::copyright_text_size;
		game_name_text_size = Settings::Splash::game_name_text_size;
		return *this;
	}
}// namespace Engine
