#include <Core/enums.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/vector.hpp>

namespace Engine::Settings
{
	extern ENGINE_EXPORT String engine_class;
	extern ENGINE_EXPORT String default_language;
	extern ENGINE_EXPORT String current_language;
	extern ENGINE_EXPORT uint_t num_threads;
	extern ENGINE_EXPORT int_t lz4_compression_level;
	extern ENGINE_EXPORT int_t gc_max_object_per_tick;
	extern ENGINE_EXPORT int_t fps_limit;
	extern ENGINE_EXPORT float screen_percentage;
	extern ENGINE_EXPORT Vector<String> languages;
	extern ENGINE_EXPORT Vector<String> systems;
	extern ENGINE_EXPORT Vector<String> plugins;
	extern ENGINE_EXPORT bool debug_shaders;

	namespace Rendering
	{
		extern ENGINE_EXPORT String rhi;
		extern ENGINE_EXPORT uint_t shadow_map_size;
		extern ENGINE_EXPORT bool force_keep_cpu_resources;
		extern ENGINE_EXPORT float anisotropy;
	}// namespace Rendering

	namespace Window
	{
		extern ENGINE_EXPORT String title;
		extern ENGINE_EXPORT String client;
		extern ENGINE_EXPORT int_t size_x;
		extern ENGINE_EXPORT int_t size_y;
		extern ENGINE_EXPORT int_t pos_x;
		extern ENGINE_EXPORT int_t pos_y;
		extern ENGINE_EXPORT bool vsync;
		extern ENGINE_EXPORT Vector<WindowAttribute> attributes;
		extern ENGINE_EXPORT Vector<Orientation> orientations;
	}// namespace Window

	namespace Splash
	{
		extern ENGINE_EXPORT bool show;
		extern ENGINE_EXPORT String image;
		extern ENGINE_EXPORT String font;
		extern ENGINE_EXPORT int_t startup_text_size;
		extern ENGINE_EXPORT int_t version_text_size;
		extern ENGINE_EXPORT int_t copyright_text_size;
		extern ENGINE_EXPORT int_t game_name_text_size;
	}// namespace Splash
}// namespace Engine::Settings
