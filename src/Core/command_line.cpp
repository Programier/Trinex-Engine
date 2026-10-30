#include <Core/command_line.hpp>

namespace Trinex
{
	Array<String> CommandLine::m_arguments;
	Array<String> CommandLine::m_positional;
	CommandLine::Options CommandLine::m_options;

	String CommandLine::normalize_name(StringView name)
	{
		if (name.starts_with("--"))
			name.remove_prefix(2);
		else if (name.starts_with('-'))
			name.remove_prefix(1);

		return String(name);
	}

	bool CommandLine::is_option(StringView argument)
	{
		return argument.size() > 1 && argument.front() == '-' && argument != "--";
	}

	void CommandLine::add_option(StringView name, StringView value, bool has_value)
	{
		String normalized_name = normalize_name(name);
		if (normalized_name.empty())
			return;

		Values& values = m_options[normalized_name];
		if (has_value)
			values.push_back(String(value));
	}

	void CommandLine::init(i32 argc, const char* const* argv)
	{
		clear();
		if (argc <= 0 || argv == nullptr)
			return;

		m_arguments.reserve(argc);
		for (i32 index = 0; index < argc; ++index)
			m_arguments.emplace_back(argv[index] ? argv[index] : "");

		bool parse_options = true;
		for (usize index = 1; index < m_arguments.size(); ++index)
		{
			const StringView argument = m_arguments[index];
			if (parse_options && argument == "--")
			{
				parse_options = false;
				continue;
			}

			if (!parse_options || !is_option(argument))
			{
				m_positional.push_back(String(argument));
				continue;
			}

			const StringView option = argument.substr(argument.starts_with("--") ? 2 : 1);
			const usize separator   = option.find('=');
			if (separator != StringView::npos)
			{
				add_option(option.substr(0, separator), option.substr(separator + 1), true);
				continue;
			}

			if (index + 1 < m_arguments.size() && m_arguments[index + 1] != "--" && !is_option(m_arguments[index + 1]))
			{
				add_option(option, m_arguments[++index], true);
				continue;
			}

			add_option(option, {}, false);
		}
	}

	void CommandLine::clear()
	{
		m_arguments.clear();
		m_positional.clear();
		m_options.clear();
	}

	const Array<String>& CommandLine::args()
	{
		return m_arguments;
	}

	const Array<String>& CommandLine::positional()
	{
		return m_positional;
	}

	bool CommandLine::has(StringView name)
	{
		return values(name) != nullptr;
	}

	StringView CommandLine::value(StringView name, StringView fallback)
	{
		const Values* option_values = values(name);
		return option_values && !option_values->empty() ? option_values->back() : fallback;
	}

	const CommandLine::Values* CommandLine::values(StringView name)
	{
		auto it = m_options.find(normalize_name(name));
		return it == m_options.end() ? nullptr : &it->second;
	}

	const CommandLine::Options& CommandLine::options()
	{
		return m_options;
	}

	void CommandLine::add(StringView name)
	{
		add_option(name, {}, false);
	}

	void CommandLine::add(StringView name, StringView value)
	{
		add_option(name, value, true);
	}
} // namespace Trinex
