#include <Core/archive.hpp>
#include <Core/etl/charconv.hpp>
#include <Core/etl/templates.hpp>
#include <ScriptEngine/script_binding.hpp>
#include <ScriptEngine/script_engine.hpp>

namespace Trinex
{
	trinex_on_pre_init({.name = "Trinex::StringView", .after = {"Trinex::DefaultScriptAddons"}})
	{
		auto flags = ScriptClassFlags::Pod | ScriptClassFlags::AppClassAllInts;
		ScriptBinding::Class registrar =
		        ScriptBinding::Class::create("Trinex::StringView", ScriptBinding::value_type<StringView>(flags));

		registrar.behaviour(ScriptClassBehave::Construct, "void f()", ScriptBinding::Helpers::constructor<StringView>);
		registrar.behaviour(ScriptClassBehave::Construct, "void f(const string& in)",
		                    ScriptBinding::Helpers::constructor<StringView, const String&>);

		registrar.method("bool empty() const", &StringView::empty);
		registrar.method("uint64 length() const", &StringView::length);
		registrar.method("uint64 size() const", &StringView::size);
		registrar.method("uint64 max_size() const", &StringView::max_size);
		registrar.method("uint8 at(uint64) const", &StringView::at);
		registrar.method("uint8 opIndex(uint64) const", &StringView::at);
		registrar.method("StringView substr(uint64 = 0, uint64 = -1) const", &StringView::substr);
		registrar.method("void remove_prefix(uint64)", &StringView::remove_prefix);
		registrar.method("void remove_suffix(uint64)", &StringView::remove_suffix);

		registrar.method("bool starts_with(uint8) const", overload_of<bool(char)>(&StringView::starts_with));
		registrar.method("bool starts_with(StringView) const", overload_of<bool(StringView)>(&StringView::starts_with));

		registrar.method("uint64 find(uint8, uint64 = 0) const",
		                 overload_of<StringView::size_type(char, StringView::size_type)>(&StringView::find));
		registrar.method("uint64 find(StringView, uint64 = 0) const",
		                 overload_of<StringView::size_type(StringView, StringView::size_type)>(&StringView::find));

		registrar.method("uint64 find_first_not_of(uint8, uint64 = 0) const",
		                 overload_of<StringView::size_type(char, StringView::size_type)>(&StringView::find_first_not_of));
		registrar.method("uint64 find_first_not_of(StringView, uint64 = 0) const",
		                 overload_of<StringView::size_type(StringView, StringView::size_type)>(&StringView::find_first_not_of));

		registrar.method("uint64 find_first_of(uint8, uint64 = 0) const",
		                 overload_of<StringView::size_type(char, StringView::size_type)>(&StringView::find_first_of));
		registrar.method("uint64 find_first_of(StringView, uint64 = 0) const",
		                 overload_of<StringView::size_type(StringView, StringView::size_type)>(&StringView::find_first_of));

		registrar.method("uint64 find_last_not_of(uint8, uint64 = 0) const",
		                 overload_of<StringView::size_type(char, StringView::size_type)>(&StringView::find_last_not_of));
		registrar.method("uint64 find_last_not_of(StringView, uint64 = 0) const",
		                 overload_of<StringView::size_type(StringView, StringView::size_type)>(&StringView::find_last_not_of));

		registrar.method("uint64 find_last_of(uint8, uint64 = 0) const",
		                 overload_of<StringView::size_type(char, StringView::size_type)>(&StringView::find_last_of));
		registrar.method("uint64 find_last_of(StringView, uint64 = 0) const",
		                 overload_of<StringView::size_type(StringView, StringView::size_type)>(&StringView::find_last_of));
	}

	trinex_api void serialize(class Archive& ar, String& string)
	{
		usize size = string.length();
		ar(size);

		if (ar.is_reading())
		{
			string.resize(size);
			ar.read(string.data(), size);
		}
		else if (ar.is_saving())
		{
			ar.write(string.data(), size);
		}
	}

	namespace Strings
	{
		template<typename T>
		static usize parse_number(StringView text, T& value)
		{
			T parsed{};
			const char* begin       = text.data();
			const auto [end, error] = etl::from_chars(begin, begin + text.size(), parsed);

			if (error != std::errc{} || end == begin)
				return 0;

			value = parsed;
			return static_cast<usize>(end - begin);
		}

		template<typename T>
		static bool format_number(String& stream, T value)
		{
			char buffer[128];
			const auto [end, error] = etl::to_chars(buffer, buffer + sizeof(buffer), value);

			if (error != std::errc{})
				return false;

			stream.append(buffer, end);
			return true;
		}

		static bool starts_with_ignore_case(StringView text, StringView value)
		{
			if (text.size() < value.size())
				return false;

			for (usize index = 0; index < value.size(); ++index)
			{
				const char left  = text[index];
				const char right = value[index];

				if (left != right && (left | 0x20) != right)
					return false;
			}

			return true;
		}

		trinex_api StringView trim(StringView value)
		{
			const auto first = value.find_first_not_of(" \t");

			if (first == std::string_view::npos)
				return {};

			const auto last = value.find_last_not_of(" \t");

			return value.substr(first, last - first + 1);
		}

		trinex_api StringView lstrip(const StringView& line, const StringView& chars)
		{
			StringView::size_type start = line.find_first_not_of(chars);
			return (start == StringView::npos) ? "" : line.substr(start, line.size() - start);
			return line;
		}

		trinex_api StringView rstrip(const StringView& line, const StringView& chars)
		{
			StringView::size_type end = line.find_last_not_of(chars);
			return end == StringView::npos ? "" : line.substr(0, end + 1);
		}

		trinex_api StringView lstrip(const StringView& line, bool (*callback)(char ch))
		{
			StringView::size_type pos = 0;
			while (pos < line.size() && callback(line[pos]))
				pos++;
			return line.substr(pos, line.length() - pos);
		}

		trinex_api StringView rstrip(const StringView& line, bool (*callback)(char ch))
		{
			StringView::size_type pos = line.size() - 1;
			while (pos > 0 && callback(line[pos]))
				pos--;
			if (pos == 0 && callback(line[pos]))
				return "";
			return line.substr(0, pos + 1);
		}

		trinex_api usize parse(StringView text, bool& value)
		{
			if (starts_with_ignore_case(text, "true"))
			{
				value = true;
				return 4;
			}

			if (starts_with_ignore_case(text, "false"))
			{
				value = false;
				return 5;
			}

			i64 numeric_value  = 0;
			const usize length = parse(text, numeric_value);

			if (length == 0)
				return 0;

			value = numeric_value != 0;
			return length;
		}

		trinex_api usize parse(StringView text, i64& value)
		{
			return parse_number(text, value);
		}

		trinex_api usize parse(StringView text, u64& value)
		{
			return parse_number(text, value);
		}

		trinex_api usize parse(StringView text, f64& value)
		{
			return parse_number(text, value);
		}

		trinex_api bool format(String& stream, bool value)
		{
			stream += value ? "true" : "false";
			return true;
		}

		trinex_api bool format(String& stream, i64 value)
		{
			return format_number(stream, value);
		}

		trinex_api bool format(String& stream, u64 value)
		{
			return format_number(stream, value);
		}

		trinex_api bool format(String& stream, f64 value)
		{
			return format_number(stream, value);
		}
	} // namespace Strings

} // namespace Trinex
