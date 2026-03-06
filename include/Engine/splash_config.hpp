#include <Core/engine_types.hpp>
#include <Core/filesystem/path.hpp>

namespace Engine
{
	struct ENGINE_EXPORT SplashConfig {
		Path image_path;
		Path font_path;
		i32 startup_text_size;
		i32 version_text_size;
		i32 copyright_text_size;
		i32 game_name_text_size;


		SplashConfig();
		SplashConfig& init();
	};
}// namespace Engine
