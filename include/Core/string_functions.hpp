#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/pair.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/vector.hpp>
#include <fmt/format.h>
#include <numeric>

namespace Engine::Strings
{
	using fmt::format;

	ENGINE_EXPORT String c_style_format(const char* text, ...);
	ENGINE_EXPORT StringView lstrip(const StringView& line, const StringView& chars = " \t\n\r");
	ENGINE_EXPORT StringView rstrip(const StringView& line, const StringView& chars = " \t\n\r");
	ENGINE_EXPORT StringView lstrip(const StringView& line, bool (*callback)(char ch));
	ENGINE_EXPORT StringView rstrip(const StringView& line, bool (*callback)(char ch));
	ENGINE_EXPORT String capitalize_words(const StringView& sentence);
	ENGINE_EXPORT String make_sentence(String line);
	ENGINE_EXPORT uint64_t hash_of(const StringView& str);

	FORCE_INLINE StringView strip(const StringView& line, const StringView& chars = " \t\n\r")
	{
		return lstrip(rstrip(line, chars), chars);
	}

	FORCE_INLINE StringView strip(const StringView& line, bool (*callback)(char ch))
	{
		return lstrip(rstrip(line, callback), callback);
	}

	ENGINE_EXPORT String replace_all(StringView line, StringView old, StringView new_line);
	ENGINE_EXPORT uint_t replace_symbol(String& str, char old_symbol, char new_symbol);
	ENGINE_EXPORT uint_t replace_symbol(char* str, char old_symbol, char new_symbol);
	ENGINE_EXPORT uint_t replace_symbol(char* str, char old_symbol, char new_symbol, uint_t len);
	ENGINE_EXPORT String to_lower(StringView line);
	ENGINE_EXPORT String to_upper(StringView line);

	ENGINE_EXPORT const char* strnstr(const char* haystack, size_t haystack_len, const char* needle, size_t needle_len);
	ENGINE_EXPORT Vector<String> split(const StringView& line, char delimiter = ' ');
	ENGINE_EXPORT Vector<String> split(const StringView& line, const StringView& delimiter);

	ENGINE_EXPORT String namespace_of(const StringView& name);
	ENGINE_EXPORT String class_name_of(const StringView& name);
	ENGINE_EXPORT StringView namespace_sv_of(const StringView& name);
	ENGINE_EXPORT StringView class_name_sv_of(const StringView& name);
	ENGINE_EXPORT String concat_scoped_name(StringView scope, StringView name);
	ENGINE_EXPORT StringView parse_name_identifier(StringView sentence, StringView* out = nullptr);
	ENGINE_EXPORT StringView parse_token(StringView expression, StringView separator, StringView* out = nullptr);


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

	ENGINE_EXPORT bool boolean_of(const char* text, size_t len = 0);
	ENGINE_EXPORT int_t integer_of(const char* text);
	ENGINE_EXPORT float float_of(const char* text);
	ENGINE_EXPORT void* pointer_of(const char* text);
	ENGINE_EXPORT bool read_line(StringView& stream, StringView& out);
	ENGINE_EXPORT bool read_line(StringView& stream, StringView& out, char separator);

	static FORCE_INLINE String make_string(const StringView& view)
	{
		return String(view);
	}

	static FORCE_INLINE String make_string(const char* line)
	{
		if (line)
			return line;
		return "";
	}

	static FORCE_INLINE StringView make_string_view(const String& str)
	{
		return StringView(str);
	}

	static FORCE_INLINE StringView make_string_view(const char* line)
	{
		if (line)
			return line;
		return "";
	}


}// namespace Engine::Strings
