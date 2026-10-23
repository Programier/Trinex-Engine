#pragma once
#include <string>
#include <string_view>

namespace Trinex
{
	using String     = std::string;
	using StringView = std::string_view;

	trinex_api void serialize(class Archive& ar, String& string);

	namespace Strings
	{
		trinex_api StringView lstrip(const StringView& line, const StringView& chars = " \t\n\r");
		trinex_api StringView rstrip(const StringView& line, const StringView& chars = " \t\n\r");
		trinex_api StringView lstrip(const StringView& line, bool (*callback)(char ch));
		trinex_api StringView rstrip(const StringView& line, bool (*callback)(char ch));

		inline StringView strip(const StringView& line, const StringView& chars = " \t\n\r")
		{
			return lstrip(rstrip(line, chars), chars);
		}

		inline StringView strip(const StringView& line, bool (*callback)(char ch))
		{
			return lstrip(rstrip(line, callback), callback);
		}

		trinex_api usize parse(StringView text, bool& value);
		trinex_api usize parse(StringView text, i64& value);
		trinex_api usize parse(StringView text, u64& value);
		trinex_api usize parse(StringView text, f64& value);

		trinex_api bool format(String& stream, bool value);
		trinex_api bool format(String& stream, i64 value);
		trinex_api bool format(String& stream, u64 value);
		trinex_api bool format(String& stream, f64 value);
	}// namespace Strings
}// namespace Trinex
