#include <Core/etl/critical_section.hpp>
#include <Core/etl/vector.hpp>
#include <Core/log.hpp>
#include <Core/types/timestamp.hpp>
#include <cstdarg>


namespace Trinex::Log
{
	namespace
	{
		struct LoggerState {
			CriticalSectionRecursive critical_section;

			Vector<Listener*> listeners;
			Level minimum_level = Level::Info;
			bool enabled        = true;
			usize sequence      = 0;

			static LoggerState* instance()
			{
				static LoggerState state;
				return &state;
			}

			template<typename Func>
			static auto lock(Func&& func)
			{
				LoggerState* state = instance();
				ScopeLock lock(state->critical_section);

				return func(state);
			}
		};

		static bool is_level_enabled(Level level, Level minimum)
		{
			return static_cast<u8>(static_cast<Level::Enum>(level)) >= static_cast<u8>(static_cast<Level::Enum>(minimum));
		}

		static std::string format_message(const char* format, va_list args)
		{
			if (format == nullptr)
				return {};

			va_list args_copy;
			va_copy(args_copy, args);

			const int required = std::vsnprintf(nullptr, 0, format, args_copy);

			va_end(args_copy);

			if (required <= 0)
				return {};

			std::string buffer(static_cast<usize>(required) + 1, '\0');

			std::vsnprintf(buffer.data(), buffer.size(), format, args);
			buffer.resize(static_cast<usize>(required));

			return buffer;
		}

		static void vlog(Level level, Category& category, const char* file, u32 line, const char* func, const char* format,
		                 va_list args)
		{
			Vector<Listener*> listeners;
			usize sequence = 0;

			const bool should_log = LoggerState::lock([&](LoggerState* state) {
				if (!state->enabled)
					return false;

				if (!category.enabled)
					return false;

				if (!is_level_enabled(level, state->minimum_level))
					return false;

				listeners = state->listeners;
				sequence  = state->sequence++;

				return true;
			});

			if (!should_log)
				return;

			std::string message = format_message(format, args);

			Record record;
			record.level     = level;
			record.category  = &category;
			record.file      = file;
			record.func      = func;
			record.line      = static_cast<i32>(line);
			record.message   = message.c_str();
			record.sequence  = sequence;
			record.timestamp = Timestamp::now().value();

			for (Listener* listener : listeners)
			{
				listener->on_log_record(record);
			}
		}
	}// namespace

	ENGINE_EXPORT Category General    = {"General", true};
	ENGINE_EXPORT Category Core       = {"Core", true};
	ENGINE_EXPORT Category Reflection = {"Reflection", true};
	ENGINE_EXPORT Category Platform   = {"Platform", true};
	ENGINE_EXPORT Category Engine     = {"Engine", true};
	ENGINE_EXPORT Category Graphics   = {"Graphics", true};
	ENGINE_EXPORT Category FileSystem = {"FileSystem", true};
	ENGINE_EXPORT Category Memory     = {"Memory", true};

	ENGINE_EXPORT Category Assets   = {"Assets", true};
	ENGINE_EXPORT Category Renderer = {"Renderer", true};
	ENGINE_EXPORT Category RHI      = {"RHI", true};

	ENGINE_EXPORT Category World     = {"World", true};
	ENGINE_EXPORT Category Actor     = {"Actor", true};
	ENGINE_EXPORT Category Physics   = {"Physics", true};
	ENGINE_EXPORT Category Audio     = {"Audio", true};
	ENGINE_EXPORT Category Input     = {"Input", true};
	ENGINE_EXPORT Category Scripting = {"Scripting", true};
	ENGINE_EXPORT Category Editor    = {"Editor", true};

#define trinex_implement_log_function(name, level)                                                                               \
	ENGINE_EXPORT void name(const char* file, u32 line, const char* func, const char* format, ...)                               \
	{                                                                                                                            \
		va_list args;                                                                                                            \
		va_start(args, format);                                                                                                  \
		vlog(Level::level, General, file, line, func, format, args);                                                             \
		va_end(args);                                                                                                            \
	}                                                                                                                            \
	ENGINE_EXPORT void name(const char* file, u32 line, const char* func, Category& category, const char* format, ...)           \
	{                                                                                                                            \
		va_list args;                                                                                                            \
		va_start(args, format);                                                                                                  \
		vlog(Level::level, category, file, line, func, format, args);                                                            \
		va_end(args);                                                                                                            \
	}

	trinex_implement_log_function(debug, Debug);
	trinex_implement_log_function(info, Info);
	trinex_implement_log_function(warning, Warning);
	trinex_implement_log_function(error, Error);
	trinex_implement_log_function(critical, Critical);


	ENGINE_EXPORT void enabled(bool value)
	{
		LoggerState::lock([&](LoggerState* state) { state->enabled = value; });
	}

	ENGINE_EXPORT bool enabled()
	{
		return LoggerState::lock([](LoggerState* state) { return state->enabled; });
	}

	ENGINE_EXPORT void minimum_level(Level level)
	{
		LoggerState::lock([&](LoggerState* state) { state->minimum_level = level; });
	}

	ENGINE_EXPORT Level minimum_level()
	{
		return LoggerState::lock([](LoggerState* state) { return state->minimum_level; });
	}

	ENGINE_EXPORT void add_listener(Listener* listener)
	{
		if (listener == nullptr)
			return;

		LoggerState::lock([&](LoggerState* state) {
			auto it = std::find(state->listeners.begin(), state->listeners.end(), listener);

			if (it == state->listeners.end())
				state->listeners.push_back(listener);
		});
	}

	ENGINE_EXPORT void remove_listener(Listener* listener)
	{
		if (listener == nullptr)
			return;

		LoggerState::lock([&](LoggerState* state) {
			auto it = std::remove(state->listeners.begin(), state->listeners.end(), listener);

			state->listeners.erase(it, state->listeners.end());
		});
	}

}// namespace Trinex::Log
