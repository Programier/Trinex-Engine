#include <Core/console.hpp>
#include <Core/etl/algorithm.hpp>
#include <Core/etl/map.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/string_functions.hpp>
#include <Core/types/path.hpp>
#include <cctype>
#include <cstdlib>

namespace Trinex::Console
{
	static const StringView s_default_config_path = "[configs]:/trinex.cfg";

	StringView type_name(ValueType type)
	{
		switch (type)
		{
			case ValueType::Boolean: return "bool";
			case ValueType::U8: return "u8";
			case ValueType::U16: return "u16";
			case ValueType::U32: return "u32";
			case ValueType::U64: return "u64";
			case ValueType::I8: return "i8";
			case ValueType::I16: return "i16";
			case ValueType::I32: return "i32";
			case ValueType::I64: return "i64";
			case ValueType::F16: return "f16";
			case ValueType::F32: return "f32";
			case ValueType::F64: return "f64";
			case ValueType::String: return "String";
			case ValueType::ReflectedEnum: return "Enum";
			case ValueType::ReflectedFlags: return "Flags";
		}

		return "Unknown";
	}

	namespace
	{
		struct Registry {
			Map<String, Entry*> entries;
		};

		struct AliasRegistry {
			Map<String, String> aliases;
		};

		struct ObserverRegistry {
			Vector<Observer*> observers;
		};

		static Registry& registry()
		{
			static Registry value;
			return value;
		}

		static AliasRegistry& alias_registry()
		{
			static AliasRegistry value;
			return value;
		}

		static ObserverRegistry& observer_registry()
		{
			static ObserverRegistry value;
			return value;
		}

		static RuntimePolicy& mutable_runtime_policy()
		{
			static RuntimePolicy value;
			return value;
		}

		static bool entry_visible(const Entry& entry)
		{
			const RuntimePolicy policy = runtime_policy();

			if (entry.type() == EntryType::Variable)
			{
				const auto& variable = static_cast<const VariableEntry&>(entry);

				if (variable.flags().all(Flags::Hidden) && !policy.show_hidden)
					return false;
				if (variable.flags().all(Flags::Cheat) && !policy.allow_cheats)
					return false;
				if (variable.flags().all(Flags::DeveloperOnly) && !policy.allow_developer_only)
					return false;
			}
			else
			{
				const auto& command = static_cast<const Command&>(entry);

				if (command.flags().all(Flags::Hidden) && !policy.show_hidden)
					return false;
				if (command.flags().all(Flags::Cheat) && !policy.allow_cheats)
					return false;
				if (command.flags().all(Flags::DeveloperOnly) && !policy.allow_developer_only)
					return false;
			}

			return true;
		}

		static String execution_block_reason(const Entry& entry)
		{
			const RuntimePolicy policy = runtime_policy();

			if (entry.type() == EntryType::Variable)
			{
				const auto& variable = static_cast<const VariableEntry&>(entry);

				if (variable.flags().all(Flags::ReadOnly))
					return Strings::format("'{}' is read-only", variable.name());
				if (variable.flags().all(Flags::Cheat) && !policy.allow_cheats)
					return Strings::format("'{}' is cheat-protected", variable.name());
				if (variable.flags().all(Flags::DeveloperOnly) && !policy.allow_developer_only)
					return Strings::format("'{}' is developer-only", variable.name());
			}
			else
			{
				const auto& command = static_cast<const Command&>(entry);

				if (command.flags().all(Flags::Cheat) && !policy.allow_cheats)
					return Strings::format("'{}' is cheat-protected", command.name());
				if (command.flags().all(Flags::DeveloperOnly) && !policy.allow_developer_only)
					return Strings::format("'{}' is developer-only", command.name());
			}

			return {};
		}

		static void notify_variable_changed(const VariableEntry& variable, StringView input)
		{
			VariableChangedEvent event{variable, input};

			for (Observer* observer : observer_registry().observers)
			{
				if (observer)
					observer->on_variable_changed(event);
			}
		}

		static void notify_command_executed(StringView input, StringView command, const String& output, bool success)
		{
			CommandExecutedEvent event{input, command, output, success};

			for (Observer* observer : observer_registry().observers)
			{
				if (observer)
					observer->on_command_executed(event);
			}
		}

		static void notify_config_loaded(StringView path, const String& output, bool success, bool persistent_only)
		{
			ConfigLoadedEvent event{String(path), output, success, persistent_only};

			for (Observer* observer : observer_registry().observers)
			{
				if (observer)
					observer->on_config_loaded(event);
			}
		}

		static void register_entry(Entry* entry)
		{
			if (entry == nullptr || entry->name().empty())
				return;

			auto& entries = registry().entries;
			auto iter     = entries.find(entry->name());

			if (iter != entries.end() && iter->second != entry)
			{
				warn_log("Console", "Duplicate console entry '%s' was replaced", entry->name().c_str());
			}

			entries[entry->name()] = entry;
		}

		static void unregister_entry(Entry* entry)
		{
			if (entry == nullptr)
				return;

			auto& entries = registry().entries;
			auto iter     = entries.find(entry->name());

			if (iter != entries.end() && iter->second == entry)
				entries.erase(iter);
		}

		static Vector<String> collect_entries(FunctionRef<bool(const Entry&)> predicate)
		{
			Vector<String> result;

			for (auto& [name, entry] : registry().entries)
			{
				(void) name;

				if (!entry_visible(*entry))
					continue;

				if (predicate(*entry))
					result.push_back(entry->name());
			}

			etl::sort(result.begin(), result.end());
			return result;
		}

		static Vector<VariableEntry*> collect_variables(FunctionRef<bool(const VariableEntry&)> predicate)
		{
			Vector<VariableEntry*> result;

			for (auto& [name, entry] : registry().entries)
			{
				(void) name;

				if (entry->type() != EntryType::Variable)
					continue;

				auto* variable = static_cast<VariableEntry*>(entry);
				if (!entry_visible(*variable))
					continue;

				if (predicate(*variable))
					result.push_back(variable);
			}

			etl::sort(result.begin(), result.end(),
			          [](const VariableEntry* a, const VariableEntry* b) { return a->name() < b->name(); });
			return result;
		}

		static bool read_identifier(StringView& input, StringView& out_identifier)
		{
			input = Strings::strip(input);

			if (input.empty())
				return false;

			const char first = input.front();

			if (!(std::isalpha(static_cast<unsigned char>(first)) || first == '_'))
				return false;

			usize index = 1;
			while (index < input.length())
			{
				const char ch = input[index];
				if (!(std::isalnum(static_cast<unsigned char>(ch)) || ch == '_'))
					break;
				++index;
			}

			out_identifier = input.substr(0, index);
			input.remove_prefix(index);
			return true;
		}

		static bool read_literal(StringView& input, StringView& out_literal, const char* delimiters = ",);\n;")
		{
			input = Strings::strip(input);

			if (input.empty())
				return false;

			const char first = input.front();
			if (first == ',' || first == ')' || first == ';' || first == '\n')
				return false;

			if (first == '"' || first == '\'')
			{
				const char quote = first;
				input.remove_prefix(1);

				usize index = 0;
				while (index < input.length())
				{
					if (input[index] == '\\' && index + 1 < input.length())
					{
						index += 2;
						continue;
					}

					if (input[index] == quote)
						break;

					++index;
				}

				out_literal = input.substr(0, index);

				if (index < input.length())
					input.remove_prefix(index + 1);
				else
					input.remove_prefix(index);

				return true;
			}

			usize index = 0;
			while (index < input.length())
			{
				const char ch = input[index];
				if (std::isspace(static_cast<unsigned char>(ch)) || StringView(delimiters).find(ch) != StringView::npos)
					break;
				++index;
			}

			if (index == 0)
				return false;

			out_literal = input.substr(0, index);
			input.remove_prefix(index);
			return true;
		}

		static bool read_parenthesized_contents(StringView& input, StringView& out_contents)
		{
			input = Strings::strip(input);

			if (input.empty() || input.front() != '(')
				return false;

			input.remove_prefix(1);

			const char* data  = input.data();
			usize length      = input.length();
			usize index       = 0;
			usize depth       = 1;
			bool in_string    = false;
			char string_quote = '\0';

			while (index < length)
			{
				const char ch = data[index];

				if (in_string)
				{
					if (ch == '\\' && index + 1 < length)
					{
						index += 2;
						continue;
					}

					if (ch == string_quote)
						in_string = false;

					++index;
					continue;
				}

				if (ch == '"' || ch == '\'')
				{
					in_string    = true;
					string_quote = ch;
					++index;
					continue;
				}

				if (ch == '(')
					++depth;
				else if (ch == ')')
				{
					--depth;
					if (depth == 0)
						break;
				}

				++index;
			}

			if (depth != 0)
				return false;

			out_contents = input.substr(0, index);
			input.remove_prefix(index + 1);
			return true;
		}

		static void skip_command_separators(StringView& input)
		{
			input = Strings::strip(input);

			while (!input.empty() && (input.front() == ';' || input.front() == '\n'))
			{
				input.remove_prefix(1);
				input = Strings::strip(input);
			}
		}

		static void skip_ignored_lines(StringView& input)
		{
			while (true)
			{
				skip_command_separators(input);
				input = Strings::strip(input);

				if (input.empty())
					return;

				if (input.starts_with("#") || input.starts_with("//"))
				{
					usize newline = input.find('\n');
					input.remove_prefix(newline == StringView::npos ? input.length() : newline + 1);
					continue;
				}

				if (input.front() == '[')
				{
					usize newline = input.find('\n');
					input.remove_prefix(newline == StringView::npos ? input.length() : newline + 1);
					continue;
				}

				return;
			}
		}

		static String quote_argument_if_needed(StringView argument)
		{
			if (argument.empty())
				return "\"\"";

			const bool need_quotes = argument.find_first_of(" \t\n\r;() ,\"'=") != StringView::npos;

			if (!need_quotes)
				return String(argument);

			String quoted;
			quoted.reserve(argument.length() + 2);
			quoted.push_back('"');

			for (char ch : argument)
			{
				if (ch == '"')
					quoted.push_back('\\');
				quoted.push_back(ch);
			}

			quoted.push_back('"');
			return quoted;
		}

		static String format_variable_state(const VariableEntry& variable)
		{
			String result = Strings::format("{} = {}", variable.name(), variable.value_to_string());

			if (!variable.is_default_value())
				result += Strings::format(" (default: {})", variable.default_value_to_string());

			return result;
		}

		static String format_alias_line(StringView name, StringView expansion);

		static String format_flags(const VariableEntry& variable)
		{
			Vector<String> flags;

			if (variable.flags().all(Flags::Persistent))
				flags.push_back("Persistent");
			if (variable.flags().all(Flags::ReadOnly))
				flags.push_back("ReadOnly");
			if (variable.flags().all(Flags::Hidden))
				flags.push_back("Hidden");
			if (variable.flags().all(Flags::Cheat))
				flags.push_back("Cheat");
			if (variable.flags().all(Flags::DeveloperOnly))
				flags.push_back("DeveloperOnly");

			if (flags.empty())
				return "None";

			return Strings::join(flags, ", ");
		}

		static String format_alias_line(StringView name, StringView expansion)
		{
			return Strings::format("[alias] {} -> {}", name, expansion);
		}
	}// namespace

	namespace Detail
	{
		bool parse_boolean(StringView raw_value, bool& out_value)
		{
			raw_value = Strings::strip(raw_value);
			if (raw_value.empty())
				return false;

			if (raw_value == "true" || raw_value == "TRUE" || raw_value == "True")
			{
				out_value = true;
				return true;
			}

			if (raw_value == "false" || raw_value == "FALSE" || raw_value == "False")
			{
				out_value = false;
				return true;
			}

			String temp      = String(raw_value);
			char* end        = nullptr;
			const long value = std::strtol(temp.c_str(), &end, 0);

			if (end == nullptr || *end != '\0')
				return false;

			out_value = value != 0;
			return true;
		}

		bool parse_signed(StringView raw_value, i64& out_value)
		{
			String temp           = String(Strings::strip(raw_value));
			char* end             = nullptr;
			const long long value = std::strtoll(temp.c_str(), &end, 0);

			if (end == nullptr || *end != '\0')
				return false;

			out_value = value;
			return true;
		}

		bool parse_unsigned(StringView raw_value, u64& out_value)
		{
			String temp                    = String(Strings::strip(raw_value));
			char* end                      = nullptr;
			const unsigned long long value = std::strtoull(temp.c_str(), &end, 0);

			if (end == nullptr || *end != '\0')
				return false;

			out_value = value;
			return true;
		}

		bool parse_floating(StringView raw_value, f64& out_value)
		{
			String temp             = String(Strings::strip(raw_value));
			char* end               = nullptr;
			const long double value = std::strtold(temp.c_str(), &end);

			if (end == nullptr || *end != '\0')
				return false;

			out_value = static_cast<f64>(value);
			return true;
		}

		bool parse_string(StringView raw_value, String& out_value)
		{
			raw_value = Strings::strip(raw_value);
			out_value.clear();
			out_value.reserve(raw_value.length());

			for (usize index = 0; index < raw_value.length(); ++index)
			{
				if (raw_value[index] == '\\' && index + 1 < raw_value.length() &&
				    (raw_value[index + 1] == '\\' || raw_value[index + 1] == '"' || raw_value[index + 1] == '\''))
				{
					out_value.push_back(raw_value[index + 1]);
					++index;
					continue;
				}

				out_value.push_back(raw_value[index]);
			}
			return true;
		}

		bool parse_reflected_enum(StringView raw_value, Refl::Enum* reflection, bool is_bitfield, u64& out_value)
		{
			if (reflection == nullptr)
				return false;

			raw_value = Strings::strip(raw_value);
			if (raw_value.empty())
				return false;

			auto find_entry = [reflection](StringView token) -> const Refl::Enum::Entry* {
				token = Strings::strip(token);
				for (const auto& entry : reflection->entries())
				{
					if (StringView(entry.name.to_string()) == token)
						return &entry;
				}
				return nullptr;
			};

			if (!is_bitfield)
			{
				if (const auto* entry = find_entry(raw_value))
				{
					out_value = static_cast<u64>(entry->value);
					return true;
				}

				return false;
			}

			if (const auto* entry = find_entry(raw_value))
			{
				out_value = static_cast<u64>(entry->value);
				return true;
			}

			u64 value = 0;
			while (!raw_value.empty())
			{
				usize separator  = raw_value.find('|');
				StringView token = separator == StringView::npos ? raw_value : raw_value.substr(0, separator);
				token            = Strings::strip(token);

				if (token.empty())
					return false;

				if (const auto* entry = find_entry(token))
				{
					value |= static_cast<u64>(entry->value);
				}
				else
				{
					u64 numeric = 0;
					if (!parse_unsigned(token, numeric))
						return false;
					value |= numeric;
				}

				if (separator == StringView::npos)
					break;

				raw_value.remove_prefix(separator + 1);
			}

			out_value = value;
			return true;
		}

		String format_boolean(bool value)
		{
			return value ? "true" : "false";
		}

		String format_signed(i64 value)
		{
			return Strings::format("{}", value);
		}

		String format_unsigned(u64 value)
		{
			return Strings::format("{}", value);
		}

		String format_floating(f64 value)
		{
			return Strings::format("{}", value);
		}

		String format_string(StringView value)
		{
			return String(value);
		}

		String format_reflected_enum(Refl::Enum* reflection, u64 value, bool is_bitfield)
		{
			if (reflection == nullptr)
				return {};

			if (const auto* entry = reflection->entry(static_cast<EnumerateType>(value)))
				return entry->name.to_string();

			if (!is_bitfield)
				return {};

			if (value == 0)
				return {};

			Vector<String> parts;
			u64 remaining = value;

			for (const auto& entry : reflection->entries())
			{
				const u64 entry_value = static_cast<u64>(entry.value);

				if (entry_value == 0 || (entry_value & (entry_value - 1)) != 0)
					continue;

				if ((remaining & entry_value) == entry_value)
				{
					parts.push_back(entry.name.to_string());
					remaining &= ~entry_value;
				}
			}

			if (parts.empty())
				return {};

			if (remaining != 0)
				parts.push_back(Strings::format("{}", remaining));

			return Strings::join(parts, "|");
		}

		String format_parse_error(StringView name, StringView raw_value)
		{
			return Strings::format("Failed to parse value '{}' for '{}'", raw_value, name);
		}

		String format_assignment(StringView name, StringView value)
		{
			return Strings::format("{} = {}", name, value);
		}
	}// namespace Detail

	Entry::Entry(StringView name, StringView description, StringView category)
	    : m_name(name), m_description(description), m_category(category)
	{
		register_entry(this);
	}

	Entry::~Entry()
	{
		unregister_entry(this);
	}

	Entry& Entry::name(StringView value)
	{
		m_name = String(value);
		return *this;
	}

	Entry& Entry::description(StringView value)
	{
		m_description = value;
		return *this;
	}

	Entry& Entry::category(StringView value)
	{
		m_category = String(value);
		return *this;
	}

	VariableEntry::VariableEntry(StringView name, StringView description, StringView category, ValueType value_type, Flags flags)
	    : Entry(name, description, category), m_value_type(value_type), m_flags(flags)
	{}

	EntryType VariableEntry::type() const
	{
		return EntryType::Variable;
	}

	Command::Command(StringView name, String (*callback)(CommandContext&), const CommandOptionalArgs& args)
	    : Entry(resolve_name(name, args.name), args.description, args.category), m_usage(args.usage), m_example(args.example),
	      m_flags(args.flags), m_parameters(args.parameters), m_callback(callback)
	{
		if (m_usage.empty() && !m_parameters.empty())
		{
			Vector<String> parts;
			parts.reserve(m_parameters.size());

			for (const auto& parameter : m_parameters)
			{
				String part = Strings::format("{}: {}", parameter.name, parameter.type);
				if (parameter.has_default_value)
					part += Strings::format(" = {}", parameter.default_value_text);
				parts.push_back(std::move(part));
			}

			m_usage = Strings::format("{}({})", this->name(), Strings::join(parts, ", "));
		}
	}

	EntryType Command::type() const
	{
		return EntryType::Command;
	}

	Entry* find(StringView name)
	{
		auto& entries = registry().entries;
		auto iter     = entries.find(String(name));

		if (iter == entries.end())
			return nullptr;

		return iter->second;
	}

	bool exists(StringView name)
	{
		return find(name) != nullptr;
	}

	RuntimePolicy runtime_policy()
	{
		return mutable_runtime_policy();
	}

	void runtime_policy(const RuntimePolicy& policy)
	{
		mutable_runtime_policy() = policy;
	}

	void add_observer(Observer* observer)
	{
		if (observer == nullptr)
			return;

		if (etl::find(observer_registry().observers.begin(), observer_registry().observers.end(), observer) ==
		    observer_registry().observers.end())
		{
			observer_registry().observers.push_back(observer);
		}
	}

	void remove_observer(Observer* observer)
	{
		auto& observers = observer_registry().observers;
		auto iter       = etl::find(observers.begin(), observers.end(), observer);
		if (iter != observers.end())
			observers.erase(iter);
	}

	void add_alias(StringView name, StringView expansion)
	{
		if (name.empty() || expansion.empty())
			return;
		alias_registry().aliases[String(name)] = String(expansion);
	}

	bool remove_alias(StringView name)
	{
		auto& map = alias_registry().aliases;
		auto iter = map.find(String(name));
		if (iter == map.end())
			return false;
		map.erase(iter);
		return true;
	}

	String find_alias(StringView name)
	{
		auto& map = alias_registry().aliases;
		auto iter = map.find(String(name));
		return iter == map.end() ? String() : iter->second;
	}

	Vector<String> aliases()
	{
		Vector<String> result;
		result.reserve(alias_registry().aliases.size());

		for (auto& [name, expansion] : alias_registry().aliases)
		{
			(void) expansion;
			result.push_back(name);
		}

		etl::sort(result.begin(), result.end());
		return result;
	}

	Vector<String> entries()
	{
		Vector<String> result = collect_entries([](const Entry&) { return true; });
		for (auto& [name, expansion] : alias_registry().aliases)
		{
			(void) expansion;
			result.push_back(name);
		}
		etl::sort(result.begin(), result.end());
		return result;
	}

	Vector<String> complete(StringView prefix)
	{
		Vector<String> result =
		        collect_entries([prefix](const Entry& entry) { return prefix.empty() || entry.name().starts_with(prefix); });
		for (auto& [name, expansion] : alias_registry().aliases)
		{
			(void) expansion;
			if (prefix.empty() || StringView(name).starts_with(prefix))
				result.push_back(name);
		}
		etl::sort(result.begin(), result.end());
		return result;
	}

	bool read_argument(StringView& command_line, StringView& out_argument)
	{
		command_line = Strings::strip(command_line);

		if (!command_line.empty() && command_line.front() == ',')
		{
			command_line.remove_prefix(1);
			command_line = Strings::strip(command_line);
		}

		return read_literal(command_line, out_argument, ",;\n");
	}

	bool has_more_arguments(StringView command_line)
	{
		command_line = Strings::strip(command_line);
		if (!command_line.empty() && command_line.front() == ',')
		{
			command_line.remove_prefix(1);
			command_line = Strings::strip(command_line);
		}

		return !command_line.empty();
	}

	StringView peek_command_tail(StringView command_line)
	{
		return read_command_tail(command_line);
	}

	StringView read_command_tail(StringView& command_line)
	{
		command_line = Strings::strip(command_line);

		if (command_line.empty())
			return {};

		if (command_line.front() == ',')
		{
			command_line.remove_prefix(1);
			command_line = Strings::strip(command_line);
		}

		StringView tail = command_line;
		command_line.remove_prefix(command_line.length());
		return Strings::strip(tail);
	}

	String Command::execute(StringView& command_line, StringView input, ExecuteFlags flags)
	{
		CommandContext context{*this, input, command_line, "", flags};
		context.values.reserve(m_parameters.size());

		for (const auto& parameter : m_parameters)
		{
			StringView raw_value;

			if (!read_argument(command_line, raw_value))
			{
				if (parameter.has_default_value)
				{
					context.values.push_back(parameter.default_value);
					continue;
				}

				context.error = Strings::format("Expected parameter '{}: {}'", parameter.name, parameter.type);
				return context.error;
			}

			Any parsed_value;
			if (parameter.parse == nullptr || !parameter.parse(raw_value, parsed_value))
			{
				context.error = Strings::format("Failed to parse parameter '{}: {}' from '{}'", parameter.name, parameter.type,
				                                raw_value);
				return context.error;
			}

			if (parameter.validate)
			{
				if (String validation = parameter.validate(parsed_value); !validation.empty())
				{
					context.error = Strings::format("Parameter '{}': {}", parameter.name, validation);
					return context.error;
				}
			}

			context.values.push_back(std::move(parsed_value));
		}

		String result = execute(context);

		if (!context.failed() && has_more_arguments(command_line))
			return Strings::format("Too many arguments for command '{}'", name());

		return result;
	}

	String Command::execute(CommandContext& ctx)
	{
		if (!m_callback)
			return Strings::format("Command '{}' has no callback", ctx.command.name());

		String result = m_callback(ctx);

		if (result.empty() && ctx.failed())
			return ctx.error;

		return result;
	}

	String execute(StringView command_line, ExecuteFlags flags)
	{
		Vector<String> results;

		skip_ignored_lines(command_line);

		while (!command_line.empty())
		{
			StringView statement_begin = command_line;
			StringView cursor          = command_line;
			StringView command;

			if (!read_identifier(cursor, command))
			{
				String error = Strings::format("Unexpected token near '{}'",
				                               command_line.substr(0, etl::min<usize>(command_line.length(), 32)));
				results.push_back(error);
				notify_command_executed(command_line, {}, error, false);
				read_command_tail(command_line);
				skip_ignored_lines(command_line);
				continue;
			}

			StringView statement = Strings::strip(cursor);
			StringView exact_input;
			auto finalize_input = [&](StringView tail) {
				exact_input = Strings::strip(statement_begin.substr(0, statement_begin.length() - tail.length()));
			};

			if (String alias = find_alias(command);
			    !alias.empty() && (statement.empty() || (statement.front() != '(' && statement.front() != '=')))
			{
				finalize_input(cursor);
				command_line = cursor;

				String result = execute(alias, flags);
				if (!result.empty())
					results.push_back(std::move(result));

				const bool success = result.find("Unknown console entry") == String::npos;
				notify_command_executed(exact_input, command, result, success);
				skip_ignored_lines(command_line);
				continue;
			}

			Entry* entry = find(command);

			if (entry == nullptr)
			{
				finalize_input(cursor);
				command_line = cursor;

				String error = Strings::format("Unknown console entry '{}'", command);
				notify_command_executed(exact_input, command, error, false);

				if (!flags.all(ExecuteFlags::IgnoreUnknown))
					results.push_back(std::move(error));

				skip_ignored_lines(command_line);
				continue;
			}

			if (!entry_visible(*entry))
			{
				finalize_input(cursor);
				command_line = cursor;

				String error = Strings::format("Console entry '{}' is not available in current runtime policy", command);
				results.push_back(error);
				notify_command_executed(exact_input, command, error, false);
				skip_ignored_lines(command_line);
				continue;
			}

			String result;

			if (entry->type() == EntryType::Variable)
			{
				if (!statement.empty() && statement.front() == '(')
				{
					finalize_input(cursor);
					command_line = cursor;
					result = Strings::format("'{}' is a console variable. Use '{}' or '{} = <value>'", command, command, command);
				}
				else
				{
					if (String error = execution_block_reason(*entry); !error.empty())
					{
						finalize_input(cursor);
						command_line = cursor;
						results.push_back(error);
						notify_command_executed(exact_input, command, error, false);
						skip_ignored_lines(command_line);
						continue;
					}

					String previous_value = static_cast<VariableEntry*>(entry)->value_to_string();
					StringView value_stream;

					if (!statement.empty() && statement.front() == '=')
					{
						value_stream = statement;
						value_stream.remove_prefix(1);
					}

					StringView probe = value_stream;
					StringView ignored_argument;
					if (read_argument(probe, ignored_argument))
						finalize_input(probe);
					else
						finalize_input(value_stream);

					result       = entry->execute(value_stream, exact_input, flags);
					command_line = value_stream;

					if (previous_value != static_cast<VariableEntry*>(entry)->value_to_string())
						notify_variable_changed(*static_cast<VariableEntry*>(entry), exact_input);
				}
			}
			else
			{
				if (String error = execution_block_reason(*entry); !error.empty())
				{
					finalize_input(cursor);
					command_line = cursor;
					results.push_back(error);
					notify_command_executed(exact_input, command, error, false);
					skip_ignored_lines(command_line);
					continue;
				}

				if (statement.empty() || statement.front() != '(')
				{
					finalize_input(cursor);
					command_line = cursor;
					result       = Strings::format("Command '{}' must be called as {}(...)", command, command);
				}
				else
				{
					StringView arguments;
					cursor = statement;

					if (!read_parenthesized_contents(cursor, arguments))
					{
						result = Strings::format("Missing closing ')' for command '{}'", command);
					}
					else
					{
						StringView argument_stream = arguments;
						finalize_input(cursor);
						result       = entry->execute(argument_stream, exact_input, flags);
						command_line = cursor;
					}

					if (exact_input.empty())
					{
						finalize_input(cursor);
						command_line = cursor;
					}
				}
			}

			if (exact_input.empty())
				finalize_input(command_line);

			if (!result.empty())
				results.push_back(std::move(result));

			const bool success = result.find("Unknown console entry") == String::npos &&
			                     result.find("Failed to ") == String::npos && result.find("Validation failed") == String::npos &&
			                     result.find("read-only") == String::npos && result.find("developer-only") == String::npos &&
			                     result.find("cheat-protected") == String::npos &&
			                     result.find("Too many arguments") == String::npos;
			notify_command_executed(exact_input, command, result, success);

			skip_ignored_lines(command_line);
		}

		return Strings::join(results, "\n");
	}

	String execute(int argc, char** argv, ExecuteFlags flags)
	{
		if (argc <= 0 || argv == nullptr)
			return {};

		StringView name = argv[0] ? StringView(argv[0]) : StringView();

		if (argc == 1)
			return execute(name, flags);

		if (argc >= 2 && argv[1] != nullptr && StringView(argv[1]) == "=")
		{
			Vector<String> value_parts;
			value_parts.reserve(argc - 2);

			for (int index = 2; index < argc; ++index) value_parts.push_back(argv[index] ? String(argv[index]) : String());

			String value = Strings::join(value_parts, " ");
			return execute(Strings::format("{} = {}", name, quote_argument_if_needed(value)), flags);
		}

		Vector<String> arguments;
		arguments.reserve(argc - 1);

		for (int index = 1; index < argc; ++index)
			arguments.push_back(quote_argument_if_needed(argv[index] ? StringView(argv[index]) : StringView()));

		return execute(Strings::format("{}({})", name, Strings::join(arguments, ", ")), flags);
	}


	//////////////////////////////// BUILDIN COMMANDS AND VARIABLES ////////////////////////////////

	trinex_static_console_command(console_help, .name = "help", .description = "Show console help", .usage = "help([name])")
	{
		StringView name;

		if (read_argument(ctx.stream, name))
		{
			if (Entry* entry = find(name))
			{
				static auto format_entry_help = [](const Entry& entry) {
					const char* type = entry.type() == EntryType::Variable ? "var" : "cmd";
					String category  = entry.category().empty() ? "" : Strings::format(" [{}]", entry.category());

					if (entry.description().empty())
						return Strings::format("[{}] {}{}", type, entry.name(), category);

					return Strings::format("[{}] {}{} - {}", type, entry.name(), category, entry.description());
				};

				String result = format_entry_help(*entry);

				if (entry->type() == EntryType::Variable)
				{
					auto* variable = static_cast<VariableEntry*>(entry);
					result += Strings::format("\nType: {}\nCurrent value: {}", type_name(variable->value_type()),
					                          variable->value_to_string());
					result += Strings::format("\nDefault value: {}\nFlags: {}", variable->default_value_to_string(),
					                          format_flags(*variable));
				}
				else
				{
					auto* command = static_cast<Command*>(entry);
					if (!command->usage().empty())
						result += Strings::format("\nUsage: {}", command->usage());
					if (!command->example().empty())
						result += Strings::format("\nExample: {}", command->example());
					if (!command->parameters().empty())
					{
						Vector<String> parameter_lines;
						parameter_lines.reserve(command->parameters().size());

						for (const auto& parameter : command->parameters())
						{
							String line = Strings::format("{}: {}", parameter.name, parameter.type);
							if (parameter.has_default_value)
								line += Strings::format(" = {}", parameter.default_value_text);
							if (!parameter.description.empty())
								line += Strings::format(" - {}", parameter.description);
							parameter_lines.push_back(std::move(line));
						}

						result += Strings::format("\nParameters:\n{}", Strings::join(parameter_lines, "\n"));
					}
				}

				return result;
			}

			if (String expansion = find_alias(name); !expansion.empty())
				return format_alias_line(name, expansion);

			return Strings::format("Unknown console entry '{}'", name);
		}

		Vector<String> lines = collect_entries([](const Entry&) { return true; });
		etl::sort(lines.begin(), lines.end());
		return Strings::join(lines, "\n");
	}

	trinex_static_console_command(console_list, .name = "list", .description = "List console entries", .usage = "list([prefix])")
	{
		StringView prefix;
		read_argument(ctx.stream, prefix);
		Vector<String> result = complete(prefix);

		if (result.empty())
			return prefix.empty() ? "No console entries registered"
			                      : Strings::format("No console entries for prefix '{}'", prefix);

		return Strings::join(result, "\n");
	}

	trinex_static_console_command(console_find, .name = "find", .description = "Search console entries", .usage = "find(<text>)")
	{
		StringView pattern_view;

		if (!read_argument(ctx.stream, pattern_view))
			return "Usage: find(<text>)";

		const String pattern  = Strings::to_lower(pattern_view);
		Vector<String> result = collect_entries([&pattern](const Entry& entry) {
			return Strings::to_lower(entry.name()).find(pattern) != String::npos ||
			       Strings::to_lower(entry.description()).find(pattern) != String::npos;
		});

		if (result.empty())
			return Strings::format("No console entries matched '{}'", pattern_view);

		return Strings::join(result, "\n");
	}

	trinex_static_console_command(console_reset, .name = "reset", .description = "Restore variable default value",
	                              .usage = "reset(<name>)")
	{
		StringView name;

		if (!read_argument(ctx.stream, name))
			return "Usage: reset(<variable>)";

		Entry* entry = find(name);

		if (entry == nullptr)
			return Strings::format("Unknown console entry '{}'", name);

		if (entry->type() != EntryType::Variable)
			return Strings::format("'{}' is not a console variable", name);

		auto* variable = static_cast<VariableEntry*>(entry);
		if (variable->flags().all(Flags::ReadOnly))
			return Strings::format("'{}' is read-only", name);
		variable->reset(ctx.input);
		return format_variable_state(*variable);
	}

	trinex_static_console_command(console_reset_all, .name = "reset_all",
	                              .description = "Restore all console variables to default values", .usage = "reset_all()")
	{
		Vector<VariableEntry*> variables = collect_variables([](const VariableEntry&) { return true; });
		usize reset_count                = 0;

		for (VariableEntry* variable : variables)
		{
			if (variable->flags().all(Flags::ReadOnly))
				continue;
			variable->reset("reset_all");
			++reset_count;
		}

		return Strings::format("Reset {} console variables", reset_count);
	}

	trinex_static_console_command(console_list_changed, .name = "list_changed",
	                              .description = "List variables changed from default values", .usage = "list_changed()")
	{
		Vector<VariableEntry*> variables =
		        collect_variables([](const VariableEntry& variable) { return !variable.is_default_value(); });

		if (variables.empty())
			return "No console variables differ from defaults";

		Vector<String> lines;
		lines.reserve(variables.size());

		for (VariableEntry* variable : variables) lines.push_back(variable->name());

		return Strings::join(lines, "\n");
	}

	trinex_static_console_command(console_diff_vars, .name = "diff_vars",
	                              .description = "Show current/default values for changed variables", .usage = "diff_vars()")
	{
		Vector<VariableEntry*> variables =
		        collect_variables([](const VariableEntry& variable) { return !variable.is_default_value(); });

		if (variables.empty())
			return "No console variables differ from defaults";

		Vector<String> lines;
		lines.reserve(variables.size());

		for (VariableEntry* variable : variables)
		{
			lines.push_back(Strings::format("{} = {} (default: {})", variable->name(), variable->value_to_string(),
			                                variable->default_value_to_string()));
		}

		return Strings::join(lines, "\n");
	}

	trinex_static_console_command(console_save_console_vars, .name = "save_console_vars",
	                              .description = "Save persistent variables to file", .usage = "save_console_vars([path])")
	{
		StringView path;

		if (!read_argument(ctx.stream, path))
			path = s_default_config_path;

		FileWriter writer(path, true);

		if (!writer.is_open())
			return Strings::format("Failed to open '{}' for writing", path);

		Vector<VariableEntry*> variables =
		        collect_variables([](const VariableEntry& variable) { return variable.flags().all(Flags::Persistent); });

		String header = "# Trinex console vars v1\n[persistent]\n";
		writer.write(reinterpret_cast<const u8*>(header.data()), header.size());

		for (VariableEntry* variable : variables)
		{
			String line = Strings::format("{} = {}\n", variable->name(), quote_argument_if_needed(variable->value_to_string()));
			writer.write(reinterpret_cast<const u8*>(line.data()), line.size());
		}

		return Strings::format("Saved {} persistent variables to '{}'", variables.size(), path);
	}

	trinex_static_console_command(console_load_console_vars, .name = "load_console_vars",
	                              .description = "Load variables from file",
	                              .usage       = "load_console_vars([path], [ignore_unknown=false])")
	{
		StringView path;
		bool ignore_unknown = false;

		if (read_argument(ctx.stream, path))
		{
			StringView flag;
			if (read_argument(ctx.stream, flag))
				Detail::parse_boolean(flag, ignore_unknown);
		}
		else
		{
			path = s_default_config_path;
		}

		FileReader reader(path);

		if (!reader.is_open())
		{
			String result = Strings::format("Failed to open '{}' for reading", path);
			notify_config_loaded(path, result, false, true);
			return result;
		}

		ExecuteFlags flags = ctx.flags;

		if (ignore_unknown)
		{
			flags = ExecuteFlags::IgnoreUnknown;
		}

		String result = execute(reader.read_string(), flags);
		notify_config_loaded(path, result, true, true);
		return result;
	}

	trinex_static_console_command(console_exec, .name = "exec", .description = "Execute console commands from file",
	                              .usage = "exec(<path>, [ignore_unknown=false])")
	{
		StringView path;
		bool ignore_unknown = false;

		if (!read_argument(ctx.stream, path))
			return "Usage: exec(<path>)";

		StringView flag;
		if (read_argument(ctx.stream, flag))
			Detail::parse_boolean(flag, ignore_unknown);

		FileReader reader(path);

		if (!reader.is_open())
		{
			String result = Strings::format("Failed to open '{}' for reading", path);
			notify_config_loaded(path, result, false, false);
			return result;
		}

		ExecuteFlags flags = ctx.flags;

		if (ignore_unknown)
		{
			flags = ExecuteFlags::IgnoreUnknown;
		}

		String result = Console::execute(reader.read_string(), flags);
		notify_config_loaded(path, result, true, false);
		return result;
	}

	trinex_static_console_command(console_alias, .name = "alias", .description = "Register or replace alias",
	                              .usage = "alias(<name>, <expansion>)")
	{
		StringView name;
		if (!read_argument(ctx.stream, name))
			return "Usage: alias(<name>, <expansion>)";

		String expansion = String(read_command_tail(ctx.stream));
		if (expansion.empty())
			return "Usage: alias(<name>, <expansion>)";

		add_alias(name, expansion);
		return format_alias_line(name, expansion);
	}

	trinex_static_console_command(console_unalias, .name = "unalias", .description = "Remove alias", .usage = "unalias(<name>)")
	{
		StringView name;
		if (!read_argument(ctx.stream, name))
			return "Usage: unalias(<name>)";

		if (!remove_alias(name))
			return Strings::format("Alias '{}' was not found", name);

		return Strings::format("Removed alias '{}'", name);
	}

	trinex_static_console_command(console_aliases, .name = "aliases", .description = "List aliases", .usage = "aliases()")
	{
		if (alias_registry().aliases.empty())
			return "No aliases registered";

		Vector<String> lines;
		lines.reserve(alias_registry().aliases.size());

		for (auto& [name, expansion] : alias_registry().aliases) lines.push_back(format_alias_line(name, expansion));

		etl::sort(lines.begin(), lines.end());
		return Strings::join(lines, "\n");
	}
}// namespace Trinex::Console
