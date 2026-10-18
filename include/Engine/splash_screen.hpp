#include <Core/enums.hpp>
#include <Core/etl/string.hpp>

namespace Trinex
{
	trinex_api void show_splash_screen();
	trinex_api void splash_screen_text(SplashTextType type, const StringView& text);
	trinex_api void hide_splash_screen();
}// namespace Trinex
