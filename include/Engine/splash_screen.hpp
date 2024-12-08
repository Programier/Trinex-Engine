#include <Core/enums.hpp>
#include <Core/etl/string.hpp>

namespace Engine
{
	ENGINE_EXPORT void show_splash_screen();
	ENGINE_EXPORT void splash_screen_text(SplashTextType type, const StringView& text);
	ENGINE_EXPORT void hide_splash_screen();
}// namespace Engine
