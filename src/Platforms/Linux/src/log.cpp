#include <cstdio>
#include <cstring>
#include <unistd.h>

namespace Trinex::Log
{
	struct Config {
		bool use_colors      = true;
		bool show_sequence   = true;
		bool show_timestamp  = true;
		bool show_file_line  = true;
		bool show_function   = false;
		bool show_category   = true;
		bool stderr_for_warn = true;
	};

	class ENGINE_EXPORT LinuxConsoleListener final : public Listener
	{
	public:
		LinuxConsoleListener(Config config = Config()) : m_config(config)
		{
			const bool stdout_is_tty = isatty(STDOUT_FILENO);
			const bool stderr_is_tty = isatty(STDERR_FILENO);

			if (!stdout_is_tty && !stderr_is_tty)
				m_config.use_colors = false;

			Trinex::Log::add_listener(this);
		}

		Listener& on_log_record(const Record& record) override
		{
			FILE* out = output_stream(record.level);

			const char* color = level_color(record.level);
			const char* reset = m_config.use_colors ? "\033[0m" : "";

			if (m_config.use_colors)
				std::fprintf(out, "%s", color);

			if (m_config.show_sequence)
				std::fprintf(out, "[%zu] ", static_cast<size_t>(record.sequence));

			if (m_config.show_timestamp)
			{
				constexpr u64 ns_per_ms = 1'000'000;
				constexpr u64 ms_per_s  = 1'000;
				constexpr u64 s_per_min = 60;
				constexpr u64 min_per_h = 60;

				const u64 total_ms = record.timestamp / ns_per_ms;
				const u64 ms       = total_ms % ms_per_s;

				const u64 total_s = total_ms / ms_per_s;
				const u64 s       = total_s % s_per_min;

				const u64 total_min = total_s / s_per_min;
				const u64 min       = total_min % min_per_h;

				const u64 h = total_min / min_per_h;

				std::fprintf(out, "[%02llu:%02llu:%02llu.%03llu]", static_cast<unsigned long long>(h),
				             static_cast<unsigned long long>(min), static_cast<unsigned long long>(s),
				             static_cast<unsigned long long>(ms));
			}

			std::fprintf(out, "[%-8s] ", level_name(record.level));

			if (m_config.show_category)
				std::fprintf(out, "[%s] ", category_name(record.category));

			std::fprintf(out, "%s", safe(record.message));

			if (m_config.show_file_line)
			{
				std::fprintf(out, "  (%s:%u", file_name(record.file), record.line);

				if (m_config.show_function && record.func != nullptr)
					std::fprintf(out, " %s", record.func);

				std::fprintf(out, ")");
			}

			std::fprintf(out, "%s\n", reset);

			if (is_error_or_higher(record.level))
				std::fflush(out);

			return *this;
		}

	private:
		Config m_config;

		FILE* output_stream(Level level) const
		{
			if (m_config.stderr_for_warn && is_warning_or_higher(level))
				return stderr;

			return stdout;
		}

		static const char* safe(const char* str) { return str ? str : ""; }

		static const char* category_name(const Category* category)
		{
			if (category == nullptr || category->name == nullptr)
				return "General";

			return category->name;
		}

		const char* level_color(Level level) const
		{
			if (!m_config.use_colors)
				return "";

			switch (level)
			{
				case Level::Debug: return "\033[90m";
				case Level::Info: return "\033[37m";
				case Level::Warning: return "\033[33m";
				case Level::Error: return "\033[31m";
				case Level::Critical: return "\033[1;31m";
				default: return "\033[37m";
			}
		}

		static const char* level_name(Level level)
		{
			switch (level)
			{
				case Level::Debug: return "Debug";
				case Level::Info: return "Info";
				case Level::Warning: return "Warning";
				case Level::Error: return "Error";
				case Level::Critical: return "Critical";
				default: return "Unknown";
			}
		}

		static const char* file_name(const char* path)
		{
			if (path == nullptr)
				return "";

			const char* slash = std::strrchr(path, '/');
			return slash ? slash + 1 : path;
		}

		static bool is_warning_or_higher(Level level)
		{
			return static_cast<u8>(static_cast<Level::Enum>(level)) >= static_cast<u8>(Level::Warning);
		}

		static bool is_error_or_higher(Level level)
		{
			return static_cast<u8>(static_cast<Level::Enum>(level)) >= static_cast<u8>(Level::Error);
		}
	} g_linux_logger;
}// namespace Trinex::Log
