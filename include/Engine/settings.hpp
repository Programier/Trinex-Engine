#include <Core/enums.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/vector.hpp>

namespace Trinex::Settings
{
	extern trinex_api String engine_class;
	extern trinex_api String default_language;
	extern trinex_api String current_language;
	extern trinex_api u32 num_threads;
	extern trinex_api i32 lz4_compression_level;
	extern trinex_api i32 gc_max_object_per_tick;
	extern trinex_api i32 fps_limit;
	extern trinex_api float screen_percentage;
	extern trinex_api Vector<String> languages;
	extern trinex_api Vector<String> systems;
	extern trinex_api Vector<String> plugins;
	extern trinex_api bool debug_shaders;

	namespace Rendering
	{
		extern trinex_api String rhi;
		extern trinex_api u32 shadow_map_size;
		extern trinex_api bool force_keep_cpu_resources;
		extern trinex_api float anisotropy;
	}// namespace Rendering

	namespace Window
	{
		extern trinex_api String title;
		extern trinex_api String client;
		extern trinex_api i32 size_x;
		extern trinex_api i32 size_y;
		extern trinex_api i32 pos_x;
		extern trinex_api i32 pos_y;
		extern trinex_api bool vsync;
		extern trinex_api Vector<WindowAttribute> attributes;
		extern trinex_api Vector<Orientation> orientations;
	}// namespace Window

	namespace Splash
	{
		extern trinex_api bool show;
		extern trinex_api String image;
		extern trinex_api String font;
		extern trinex_api i32 startup_text_size;
		extern trinex_api i32 version_text_size;
		extern trinex_api i32 copyright_text_size;
		extern trinex_api i32 game_name_text_size;
	}// namespace Splash
}// namespace Trinex::Settings
