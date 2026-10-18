#include <Core/etl/array.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Core/window.hpp>
#include <Engine/splash_config.hpp>
#include <Engine/splash_screen.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/texture.hpp>
#include <Platform/platform.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	trinex_api void show_splash_screen() {}
	trinex_api void splash_screen_text(SplashTextType type, const StringView& text) {}
	trinex_api void stop_thread(Thread*& thread) {}
	trinex_api void hide_splash_screen() {}
}// namespace Trinex
