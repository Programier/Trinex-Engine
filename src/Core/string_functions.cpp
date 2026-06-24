#include <Core/constants.hpp>
#include <Core/etl/algorithm.hpp>
#include <Core/etl/charconv.hpp>
#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Core/string_functions.hpp>
#include <regex>
#include <stdarg.h>
#include <string>


namespace Trinex::Strings
{
	ENGINE_EXPORT String c_style_format(const char* text, ...)
	{
		va_list args;
		va_start(args, text);

		char buffer[1024];
		std::vsprintf(buffer, text, args);
		va_end(args);
		return buffer;
	}

	ENGINE_EXPORT StringView lstrip(const StringView& line, const StringView& chars)
	{
		StringView::size_type start = line.find_first_not_of(chars);
		return (start == StringView::npos) ? "" : line.substr(start, line.size() - start);
		return line;
	}

	ENGINE_EXPORT StringView rstrip(const StringView& line, const StringView& chars)
	{
		StringView::size_type end = line.find_last_not_of(chars);
		return end == StringView::npos ? "" : line.substr(0, end + 1);
	}

	ENGINE_EXPORT StringView lstrip(const StringView& line, bool (*callback)(char ch))
	{
		StringView::size_type pos = 0;
		while (pos < line.size() && callback(line[pos])) pos++;
		return line.substr(pos, line.length() - pos);
	}

	ENGINE_EXPORT StringView rstrip(const StringView& line, bool (*callback)(char ch))
	{
		StringView::size_type pos = line.size() - 1;
		while (pos > 0 && callback(line[pos])) pos--;
		if (pos == 0 && callback(line[pos]))
			return "";
		return line.substr(0, pos + 1);
	}

	ENGINE_EXPORT String capitalize_words(const StringView& sentence)
	{
		std::regex word_regex(R"(\b[^\W\d_]*([A-Za-z])[^\W\d_]*\b)");
		String result = String(sentence);

		auto words_begin = std::sregex_iterator(result.begin(), result.end(), word_regex);
		auto words_end   = std::sregex_iterator();

		for (std::sregex_iterator i = words_begin; i != words_end; ++i)
		{
			auto pos    = i->position();
			result[pos] = std::toupper(result[pos]);
			std::transform(result.begin() + pos + 1, result.begin() + pos + i->length(), result.begin() + pos + 1,
			               [](char ch) { return std::tolower(ch); });
		}

		return result;
	}

	ENGINE_EXPORT String make_sentence(String line)
	{
		String temp = std::regex_replace(line, std::regex(R"(^[msgkp]_)"), "");
		temp        = std::regex_replace(temp, std::regex("([a-z])([A-Z0-9])"), "$1 $2");
		temp        = std::regex_replace(temp, std::regex("([A-Z])([A-Z0-9][a-z])"), "$1 $2");
		temp        = std::regex_replace(temp, std::regex("[-_]"), " ");
		return capitalize_words(temp);
	}

	ENGINE_EXPORT u64 hash_of(const StringView& str)
	{
		return memory_hash(str.data(), str.length(), 0);
	}

	String replace_all(StringView line, StringView old, StringView new_line)
	{
		String result;
		usize pos     = 0;
		usize old_len = old.length();

		while ((pos = line.find(old)) != StringView::npos)
		{
			result += line.substr(0, pos);
			result += new_line;
			line.remove_prefix(pos + old_len);
		}

		result += line;
		return result;
	}

	ENGINE_EXPORT u32 replace_symbol(String& str, char old_symbol, char new_symbol)
	{
		u32 count = 0;

		for (char& ch : str)
		{
			if (ch == old_symbol)
			{
				++count;
				ch = new_symbol;
			}
		}

		return count;
	}

	ENGINE_EXPORT u32 replace_symbol(char* str, char old_symbol, char new_symbol)
	{
		u32 count = 0;

		while (*str != '\0')
		{
			if (*str == old_symbol)
			{
				++count;
				*str = new_symbol;
			}

			++str;
		}

		return count;
	}

	ENGINE_EXPORT u32 replace_symbol(char* str, char old_symbol, char new_symbol, u32 len)
	{
		u32 count = 0;

		while (len > 0)
		{
			if (*str == old_symbol)
			{
				++count;
				*str = new_symbol;
			}

			++str;
			--len;
		}

		return count;
	}

	ENGINE_EXPORT String to_lower(StringView line)
	{
		String result;
		result.resize(line.size());
		etl::transform(line.begin(), line.end(), result.begin(), [](char ch) -> char { return std::tolower(ch); });
		return result;
	}

	ENGINE_EXPORT String to_upper(StringView line)
	{
		String result;
		result.resize(line.size());
		etl::transform(line.begin(), line.end(), result.begin(), [](char ch) -> char { return std::toupper(ch); });
		return result;
	}

	ENGINE_EXPORT const char* strnstr(const char* haystack, usize haystack_len, const char* needle, usize needle_len)
	{
		return reinterpret_cast<const char*>(memory_search(reinterpret_cast<const u8*>(haystack), haystack_len,
		                                                   reinterpret_cast<const u8*>(needle), needle_len));
	}


	template<typename DelimiterType>
	static FORCE_INLINE Vector<String> internal_split(const StringView& line, DelimiterType delimiter)
	{
		Vector<String> tokens;
		String token;

		usize start = 0;
		usize end   = 0;

		while ((end = line.find(delimiter, start)) != String::npos)
		{
			tokens.emplace_back(line.substr(start, end - start));
			start = end + 1;
		}

		tokens.emplace_back(line.substr(start, end));
		return tokens;
	}

	ENGINE_EXPORT Vector<String> split(const StringView& line, char delimiter)
	{
		return internal_split<char>(line, delimiter);
	}

	ENGINE_EXPORT Vector<String> split(const StringView& line, const StringView& delimiter)
	{
		return internal_split<const StringView&>(line, delimiter);
	}

	ENGINE_EXPORT String namespace_of(const StringView& name)
	{
		return String(namespace_sv_of(name));
	}

	ENGINE_EXPORT String class_name_of(const StringView& name)
	{
		return String(class_name_sv_of(name));
	}

	ENGINE_EXPORT StringView namespace_sv_of(const StringView& name)
	{
		auto index = name.find_last_of("::");
		if (index == String::npos)
			return StringView();

		index -= 1;
		return name.substr(0, index);
	}

	ENGINE_EXPORT StringView class_name_sv_of(const StringView& name)
	{
		auto pos = name.find_last_of("::");
		if (pos == String::npos)
		{
			return name;
		}

		return name.substr(pos + 1, name.length() - pos + 1);
	}

	ENGINE_EXPORT String concat_scoped_name(StringView scope, StringView name)
	{
		if (scope.empty())
			return String(name);
		return format("{}::{}", scope, name);
	}

	ENGINE_EXPORT StringView parse_name_identifier(StringView sentence, StringView* out)
	{
		return parse_token(sentence, Constants::name_separator, out);
	}

	ENGINE_EXPORT StringView parse_token(StringView expression, StringView separator, StringView* out)
	{
		auto pos = expression.find(separator);

		if (pos == StringView::npos)
		{
			if (out)
			{
				*out = StringView(expression.data() + expression.size(), 0);
			}

			return expression;
		}

		StringView name = expression.substr(0, pos);

		if (out)
		{
			expression.remove_prefix(name.length() + separator.length());
			*out = expression;
		}

		return name;
	}

	bool iequals(StringView first, StringView second)
	{
		if (first.size() != second.size())
			return false;

		return etl::equal(first.begin(), first.end(), second.begin(),
		                  [](char ca, char cb) { return std::tolower(ca) == std::tolower(cb); });
	}

	ENGINE_EXPORT bool boolean_of(StringView text)
	{
		bool result = false;
		boolean_of(text, result);
		return result;
	}

	ENGINE_EXPORT i64 signed_of(StringView text)
	{
		i64 result = 0;
		signed_of(text, result);
		return result;
	}

	ENGINE_EXPORT u64 unsigned_of(StringView text)
	{
		u64 result = 0;
		unsigned_of(text, result);
		return result;
	}

	ENGINE_EXPORT f64 floating_of(StringView text)
	{
		f64 result = 0.0;
		floating_of(text, result);
		return result;
	}

	ENGINE_EXPORT void* pointer_of(StringView text)
	{
		void* result = nullptr;
		pointer_of(text, result);
		return result;
	}

	ENGINE_EXPORT bool boolean_of(StringView text, bool& out)
	{
		if (iequals(text, "true"))
		{
			out = true;
			return true;
		}

		if (iequals(text, "false"))
		{
			out = false;
			return true;
		}

		i64 value = 0;

		if (!signed_of(text, value))
			return false;

		out = static_cast<bool>(value);
		return true;
	}

	ENGINE_EXPORT bool signed_of(StringView text, i64& out)
	{
		const char* begin = text.data();
		const char* end   = begin + text.size();

		i64 result = 0;

		auto [ptr, ec] = etl::from_chars(begin, end, result, 10);

		if (ec != std::errc{} || ptr != end)
			return false;

		out = result;
		return true;
	}

	ENGINE_EXPORT bool unsigned_of(StringView text, u64& out)
	{
		const char* begin = text.data();
		const char* end   = begin + text.size();

		u64 result = 0;

		auto [ptr, ec] = etl::from_chars(begin, end, result, 10);

		if (ec != std::errc{} || ptr != end)
			return false;

		out = result;
		return true;
	}

	ENGINE_EXPORT bool floating_of(StringView text, f64& out)
	{
		const char* begin = text.data();
		const char* end   = begin + text.size();

		f64 result = 0.0;

		auto [ptr, ec] = etl::from_chars(begin, end, result);

		if (ec != std::errc{} || ptr != end)
			return false;

		out = result;
		return true;
	}

	ENGINE_EXPORT bool pointer_of(StringView text, void*& out)
	{
		const char* begin = text.data();
		const char* end   = begin + text.size();

		if (begin == end)
			return false;

		if (end - begin >= 2 && begin[0] == '0' && (begin[1] == 'x' || begin[1] == 'X'))
		{
			begin += 2;

			if (begin == end)
				return false;
		}

		usize address = 0;

		auto [ptr, ec] = etl::from_chars(begin, end, address, 16);

		if (ec != std::errc{} || ptr != end)
			return false;

		out = reinterpret_cast<void*>(address);
		return true;
	}

	ENGINE_EXPORT bool read_line(StringView& stream, StringView& out)
	{
		if (stream.empty())
			return false;

		if (read_line(stream, out, '\n'))
			return true;

		out = stream;
		stream.remove_prefix(stream.size());
		return true;
	}

	ENGINE_EXPORT bool read_line(StringView& stream, StringView& out, char separator)
	{
		usize pos = stream.find(separator);

		if (pos == StringView::npos)
		{
			return false;
		}

		out = stream.substr(0, pos);
		stream.remove_prefix(pos + 1);
		return true;
	}
}// namespace Trinex::Strings
