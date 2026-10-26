#pragma once
#include <Core/etl/array.hpp>
#include <Core/etl/pair.hpp>
#include <Core/etl/string.hpp>
#include <fmt/format.h>

namespace Trinex::Strings
{
	using fmt::format;

	trinex_api String capitalize_words(const StringView& sentence);
	trinex_api String make_sentence(String line);
	trinex_api u64 hash_of(const StringView& str);

	trinex_api String replace_all(StringView line, StringView old, StringView new_line);
	trinex_api u32 replace_symbol(String& str, char old_symbol, char new_symbol);
	trinex_api u32 replace_symbol(char* str, char old_symbol, char new_symbol);
	trinex_api u32 replace_symbol(char* str, char old_symbol, char new_symbol, u32 len);
	trinex_api String to_lower(StringView line);
	trinex_api String to_upper(StringView line);

	trinex_api const char* strnstr(const char* haystack, usize haystack_len, const char* needle, usize needle_len);

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
} // namespace Trinex::Strings
