#pragma once

namespace Trinex::Log
{
	struct ENGINE_EXPORT Category {
		const char* name;
		bool enabled;

		constexpr Category(const char* category_name = "General", bool is_enabled = true) noexcept
		    : name(category_name), enabled(is_enabled)
		{}

		constexpr explicit operator bool() const noexcept { return enabled; }
	};

	struct Level {
		enum Enum : u8
		{
			Debug    = 0,
			Info     = 1,
			Warning  = 2,
			Error    = 3,
			Critical = 4,
		};

		trinex_enum_struct(Level);
	};

	struct ENGINE_EXPORT Record {
		Category* category;
		const char* file;
		const char* func;
		const char* message;
		u64 timestamp;
		usize sequence;
		u32 line;
		Level level;
	};

	class ENGINE_EXPORT Listener
	{
	public:
		virtual ~Listener()                                   = default;
		virtual Listener& on_log_record(const Record& record) = 0;
	};

	ENGINE_EXPORT extern Category General;
	ENGINE_EXPORT extern Category Core;
	ENGINE_EXPORT extern Category Reflection;
	ENGINE_EXPORT extern Category Platform;
	ENGINE_EXPORT extern Category Engine;
	ENGINE_EXPORT extern Category Graphics;
	ENGINE_EXPORT extern Category FileSystem;
	ENGINE_EXPORT extern Category Memory;

	ENGINE_EXPORT extern Category Assets;
	ENGINE_EXPORT extern Category Renderer;
	ENGINE_EXPORT extern Category RHI;

	ENGINE_EXPORT extern Category World;
	ENGINE_EXPORT extern Category Actor;
	ENGINE_EXPORT extern Category Physics;
	ENGINE_EXPORT extern Category Audio;
	ENGINE_EXPORT extern Category Input;
	ENGINE_EXPORT extern Category Scripting;
	ENGINE_EXPORT extern Category Editor;

	ENGINE_EXPORT void debug(const char* file, u32 line, const char* func, const char* format, ...);
	ENGINE_EXPORT void debug(const char* file, u32 line, const char* func, Category& category, const char* format, ...);

	ENGINE_EXPORT void info(const char* file, u32 line, const char* func, const char* format, ...);
	ENGINE_EXPORT void info(const char* file, u32 line, const char* func, Category& category, const char* format, ...);

	ENGINE_EXPORT void warning(const char* file, u32 line, const char* func, const char* format, ...);
	ENGINE_EXPORT void warning(const char* file, u32 line, const char* func, Category& category, const char* format, ...);

	ENGINE_EXPORT void error(const char* file, u32 line, const char* func, const char* format, ...);
	ENGINE_EXPORT void error(const char* file, u32 line, const char* func, Category& category, const char* format, ...);

	ENGINE_EXPORT void critical(const char* file, u32 line, const char* func, const char* format, ...);
	ENGINE_EXPORT void critical(const char* file, u32 line, const char* func, Category& category, const char* format, ...);

	ENGINE_EXPORT void enabled(bool enabled);
	ENGINE_EXPORT bool enabled();

	ENGINE_EXPORT void minimum_level(Level level);
	ENGINE_EXPORT Level minimum_level();

	ENGINE_EXPORT void add_listener(Listener* listener);
	ENGINE_EXPORT void remove_listener(Listener* listener);
}// namespace Trinex::Log

#if TRINEX_DEBUG_BUILD
#define trinex_debug(...) Trinex::Log::debug(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#else
#define trinex_debug(...) ((void) 0)
#endif

#define trinex_info(...) Trinex::Log::info(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#define trinex_warning(...) Trinex::Log::warning(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#define trinex_error(...) Trinex::Log::error(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define trinex_critical(...) Trinex::Log::critical(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
