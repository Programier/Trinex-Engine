#pragma once
#include <Core/export.hpp>
#include <cstdarg>

namespace Engine
{
	class ENGINE_EXPORT Logger
	{
	public:
		static Logger* logger;

		Logger& log(const char* tag, const char* format, ...);
		Logger& debug(const char* tag, const char* format, ...);
		Logger& warning(const char* tag, const char* format, ...);
		Logger& error(const char* tag, const char* format, ...);

		virtual Logger& log_msg(const char* tag, const char* msg);
		virtual Logger& debug_msg(const char* tag, const char* msg);
		virtual Logger& warning_msg(const char* tag, const char* msg);
		virtual Logger& error_msg(const char* tag, const char* msg);

		static Logger* null();
		static Logger* standart();
	};

#if TRINEX_DEBUG_BUILD
#define debug_log(tag, format, ...) Engine::Logger::logger->debug(tag, format __VA_OPT__(, ##__VA_ARGS__))
#else
#define debug_log(tag, format, ...)
#endif
#define warn_log(tag, format, ...) Engine::Logger::logger->warning(tag, format __VA_OPT__(, ##__VA_ARGS__))
#define info_log(tag, format, ...) Engine::Logger::logger->log(tag, format __VA_OPT__(, ##__VA_ARGS__))
#define error_log(tag, format, ...) Engine::Logger::logger->error(tag, format __VA_OPT__(, ##__VA_ARGS__))
}// namespace Engine
