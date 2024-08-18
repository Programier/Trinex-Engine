#include <Core/exception.hpp>
#include <Core/stacktrace.hpp>
#include <Core/string_functions.hpp>

namespace Engine
{
	static inline String get_message(const String& msg, int line, const String& file, const String& function)
	{
		StackTrace st(3);
		return Strings::format("Critical error at:"
							   "\n\tFile:\t\t{}"
							   "\n\tFunction:\t{}"
							   "\n\tLine:\t\t{}"
							   "\n\tMessage:\t{}"
							   "\n\tCallStack:\n\t\t\t{}",
							   file, function, line, msg, st.to_string(false, "\n\t\t\t"));
	}

	EngineException::EngineException(const String& msg, int line, const String& file, const String& function)
		: std::runtime_error(get_message(msg, line, file, function))
	{}
}// namespace Engine
