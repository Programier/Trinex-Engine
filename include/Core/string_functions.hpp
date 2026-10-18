#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/pair.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/vector.hpp>
#include <fmt/format.h>
#include <numeric>

namespace Trinex::Strings
{
	using fmt::format;

	trinex_api String c_style_format(const char* text, ...);
	trinex_api StringView lstrip(const StringView& line, const StringView& chars = " \t\n\r");
	trinex_api StringView rstrip(const StringView& line, const StringView& chars = " \t\n\r");
	trinex_api StringView lstrip(const StringView& line, bool (*callback)(char ch));
	trinex_api StringView rstrip(const StringView& line, bool (*callback)(char ch));
	trinex_api String capitalize_words(const StringView& sentence);
	trinex_api String make_sentence(String line);
	trinex_api u64 hash_of(const StringView& str);

	inline StringView strip(const StringView& line, const StringView& chars = " \t\n\r")
	{
		return lstrip(rstrip(line, chars), chars);
	}

	inline StringView strip(const StringView& line, bool (*callback)(char ch))
	{
		return lstrip(rstrip(line, callback), callback);
	}

	trinex_api String replace_all(StringView line, StringView old, StringView new_line);
	trinex_api u32 replace_symbol(String& str, char old_symbol, char new_symbol);
	trinex_api u32 replace_symbol(char* str, char old_symbol, char new_symbol);
	trinex_api u32 replace_symbol(char* str, char old_symbol, char new_symbol, u32 len);
	trinex_api String to_lower(StringView line);
	trinex_api String to_upper(StringView line);

	trinex_api const char* strnstr(const char* haystack, usize haystack_len, const char* needle, usize needle_len);
	trinex_api Vector<String> split(const StringView& line, char delimiter = ' ');
	trinex_api Vector<String> split(const StringView& line, const StringView& delimiter);

	trinex_api String namespace_of(const StringView& name);
	trinex_api String class_name_of(const StringView& name);
	trinex_api StringView namespace_sv_of(const StringView& name);
	trinex_api StringView class_name_sv_of(const StringView& name);
	trinex_api String concat_scoped_name(StringView scope, StringView name);
	trinex_api StringView parse_name_identifier(StringView sentence, StringView* out = nullptr);
	trinex_api StringView parse_token(StringView expression, StringView separator, StringView* out = nullptr);

	trinex_api bool iequals(StringView first, StringView second);

	trinex_api bool boolean_of(StringView text);
	trinex_api i64 signed_of(StringView text);
	trinex_api u64 unsigned_of(StringView text);
	trinex_api f64 floating_of(StringView text);
	trinex_api void* pointer_of(StringView text);

	trinex_api bool boolean_of(StringView text, bool& out);
	trinex_api bool signed_of(StringView text, i64& out);
	trinex_api bool unsigned_of(StringView text, u64& out);
	trinex_api bool floating_of(StringView text, f64& out);
	trinex_api bool pointer_of(StringView text, void*& out);

	trinex_api bool read_line(StringView& stream, StringView& out);
	trinex_api bool read_line(StringView& stream, StringView& out, char separator);


	template<typename Range, typename Value = typename Range::value_type, typename FormatFunction>
	inline String join(const Range& elements, const String& delimiter, const FormatFunction& function)
	{
		if (elements.empty())
			return "";

		return std::accumulate(std::next(elements.begin()), elements.end(), format("{}", function(elements[0])),
		                       [&delimiter, &function](const String& a, const Value& b) { return a + delimiter + function(b); });
	}

	template<typename Range, typename Value = typename Range::value_type>
	inline String join(const Range& elements, const String& delimiter)
	{
		static auto callback = [](const Value& value) {
			if constexpr (std::is_same_v<String, Value>)
			{
				return value;
			}
			else if constexpr (std::is_same_v<StringView, Value>)
			{
				return String(value);
			}
			else
			{
				return format("{}", value);
			}
		};

		return join(elements, delimiter, callback);
	}

	static inline String make_string(const StringView& view)
	{
		return String(view);
	}

	static inline String make_string(const char* line)
	{
		if (line)
			return line;
		return "";
	}

	static inline StringView make_string_view(const String& str)
	{
		return StringView(str);
	}

	static inline StringView make_string_view(const char* line)
	{
		if (line)
			return line;
		return "";
	}


}// namespace Trinex::Strings
