module;
#include <Core/etl/std.hpp>
export module Trinex.Core.ETL:String;
export import Trinex.Core.Types;

export namespace Trinex
{
	using String     = std::string;
	using StringView = std::string_view;

	using std::operator+;
	using std::operator==;
	using std::operator<=>;
	using std::operator<<;
	using std::operator>>;

	trinex_api void serialize(class Archive& ar, String& string);

	namespace Strings
	{
		trinex_api StringView trim(std::string_view value);
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

		inline StringView split(StringView& input, StringView separator)
		{
			const StringView::size_type position = separator.empty() ? StringView::npos : input.find(separator);
			const StringView result              = input.substr(0, position);

			input = position == StringView::npos ? StringView() : input.substr(position + separator.size());
			return result;
		}

		template<typename... Args>
		bool format_to(String& stream, std::format_string<Args...> fmt, Args&&... args)
		{
			std::format_to(std::back_inserter(stream), fmt, std::forward<Args>(args)...);

			return true;
		}

		template<typename... Args>
		String format(std::format_string<Args...> fmt, Args&&... args)
		{
			return std::format(fmt, std::forward<Args>(args)...);
		}

		trinex_api usize parse(StringView text, bool& value);
		trinex_api usize parse(StringView text, i64& value);
		trinex_api usize parse(StringView text, u64& value);
		trinex_api usize parse(StringView text, f64& value);

		trinex_api bool format(String& stream, bool value);
		trinex_api bool format(String& stream, i64 value);
		trinex_api bool format(String& stream, u64 value);
		trinex_api bool format(String& stream, f64 value);
	} // namespace Strings
} // namespace Trinex
