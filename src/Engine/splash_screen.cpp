#include <Core/base_engine.hpp>
#include <Core/config_manager.hpp>
#include <Core/etl/array.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Core/file_manager.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Engine/splash_config.hpp>
#include <Engine/splash_screen.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/texture_2D.hpp>
#include <Platform/platform.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>

namespace Engine
{
	ENGINE_EXPORT void show_splash_screen() {}
	ENGINE_EXPORT void splash_screen_text(SplashTextType type, const StringView& text) {}
	ENGINE_EXPORT void stop_thread(Thread*& thread) {}
	ENGINE_EXPORT void hide_splash_screen() {}
}// namespace Engine
