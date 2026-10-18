#pragma once

namespace Trinex::Log
{
	struct trinex_api Category {
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

	struct trinex_api Record {
		Category* category;
		const char* file;
		const char* func;
		const char* message;
		u64 timestamp;
		usize sequence;
		u32 line;
		Level level;
	};

	class trinex_api Listener
	{
	public:
		virtual ~Listener()                                   = default;
		virtual Listener& on_log_record(const Record& record) = 0;
	};

	trinex_api extern Category General;
	trinex_api extern Category Core;
	trinex_api extern Category Reflection;
	trinex_api extern Category Platform;
	trinex_api extern Category Engine;
	trinex_api extern Category Graphics;
	trinex_api extern Category FileSystem;
	trinex_api extern Category Memory;

	trinex_api extern Category Assets;
	trinex_api extern Category Renderer;
	trinex_api extern Category RHI;

	trinex_api extern Category World;
	trinex_api extern Category Actor;
	trinex_api extern Category Physics;
	trinex_api extern Category Audio;
	trinex_api extern Category Input;
	trinex_api extern Category Scripting;
	trinex_api extern Category Editor;

	trinex_api void debug(const char* file, u32 line, const char* func, const char* format, ...);
	trinex_api void debug(const char* file, u32 line, const char* func, Category& category, const char* format, ...);

	trinex_api void info(const char* file, u32 line, const char* func, const char* format, ...);
	trinex_api void info(const char* file, u32 line, const char* func, Category& category, const char* format, ...);

	trinex_api void warning(const char* file, u32 line, const char* func, const char* format, ...);
	trinex_api void warning(const char* file, u32 line, const char* func, Category& category, const char* format, ...);

	trinex_api void error(const char* file, u32 line, const char* func, const char* format, ...);
	trinex_api void error(const char* file, u32 line, const char* func, Category& category, const char* format, ...);

	trinex_api void critical(const char* file, u32 line, const char* func, const char* format, ...);
	trinex_api void critical(const char* file, u32 line, const char* func, Category& category, const char* format, ...);

	trinex_api void enabled(bool enabled);
	trinex_api bool enabled();

	trinex_api void minimum_level(Level level);
	trinex_api Level minimum_level();

	trinex_api void add_listener(Listener* listener);
	trinex_api void remove_listener(Listener* listener);
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
