#pragma once
#include <Core/etl/array.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/string.hpp>

namespace Trinex
{
	class trinex_api CommandLine
	{
	public:
		using Values  = Array<String>;
		using Options = Map<String, Values>;

	private:
		static String normalize_name(StringView name);
		static bool is_option(StringView argument);
		static void add_option(StringView name, StringView value, bool has_value);

		static Array<String> m_arguments;
		static Array<String> m_positional;
		static Options m_options;

	public:
		static void init(i32 argc, const char* const* argv);
		static void clear();

		static const Array<String>& args();
		static const Array<String>& positional();
		static const Options& options();

		static bool has(StringView name);
		static StringView value(StringView name, StringView fallback = "");
		static const Values* values(StringView name);

		static void add(StringView name);
		static void add(StringView name, StringView value);
	};
} // namespace Trinex
