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

	namespace
	{
		using ExecutionReport = ExecuteResult;

		struct NestedExecuteError {
			bool active          = false;
			ExecuteStatus status = ExecuteStatus::Success;
			String message;
		};

		struct ConsoleState {
			Map<String, Entry*> entries;
			Map<String, String> aliases;
			Vector<Observer*> observers;
			Vector<struct StackFrame*> stack;
			u128 stack_location;

			static ConsoleState& instance()
			{
				static ConsoleState state;
				return state;
			}

			inline void begin_execution(StackFrame* frame)
			{
				if (stack.empty())
				{
					stack_location = StackByteAllocator::location();
				}

				stack.push_back(frame);
			}

			inline void end_execution(StackFrame* frame)
			{
				trinex_assert(!stack.empty() && stack.back() == frame);
				stack.pop_back();

				if (stack.empty())
				{
					StackByteAllocator::location(stack_location);
				}
			}
		};

		static NestedExecuteError& nested_execute_error()
		{
			thread_local NestedExecuteError error;
			return error;
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

		static ExecuteStatus execution_block_status(const Entry& entry)
		{
			const RuntimePolicy policy = runtime_policy();

			if (entry.type() == EntryType::Variable)
			{
				const auto& variable = static_cast<const VariableEntry&>(entry);

				if (variable.flags().all(Flags::ReadOnly))
					return ExecuteStatus::ReadOnly;
				if (variable.flags().all(Flags::Cheat) && !policy.allow_cheats)
					return ExecuteStatus::CheatProtected;
				if (variable.flags().all(Flags::DeveloperOnly) && !policy.allow_developer_only)
					return ExecuteStatus::DeveloperOnly;
			}
			else
			{
				const auto& command = static_cast<const Command&>(entry);

				if (command.flags().all(Flags::Cheat) && !policy.allow_cheats)
					return ExecuteStatus::CheatProtected;
				if (command.flags().all(Flags::DeveloperOnly) && !policy.allow_developer_only)
					return ExecuteStatus::DeveloperOnly;
			}

			return ExecuteStatus::Success;
		}

		static void notify_command_executed(StringView input, StringView command, const String& output, ExecuteStatus status)
		{
			CommandExecutedEvent event{input, command, output, status};

			for (Observer* observer : ConsoleState::instance().observers)
			{
				if (observer)
					observer->on_command_executed(event);
			}
		}

		static void notify_config_loaded(StringView path, const String& output, ExecuteStatus status, bool persistent_only)
		{
			ConfigLoadedEvent event{String(path), output, status, persistent_only};

			for (Observer* observer : ConsoleState::instance().observers)
			{
				if (observer)
					observer->on_config_loaded(event);
			}
		}

		static void register_entry(Entry* entry)
		{
			if (entry == nullptr || entry->name().empty())
				return;

			auto& entries = ConsoleState::instance().entries;
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

			auto& entries = ConsoleState::instance().entries;
			auto iter     = entries.find(entry->name());

			if (iter != entries.end() && iter->second == entry)
				entries.erase(iter);
		}

		static Vector<String> collect_entries(FunctionRef<bool(const Entry&)> predicate)
		{
			Vector<String> result;

			for (auto& [name, entry] : ConsoleState::instance().entries)
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

			for (auto& [name, entry] : ConsoleState::instance().entries)
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

		static StringView consume_statement_tail(StringView& input)
		{
			input = Strings::strip(input);

			const char* data  = input.data();
			usize length      = input.length();
			usize index       = 0;
			usize paren_depth = 0;
			usize brace_depth = 0;
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
					++paren_depth;
				else if (ch == ')')
				{
					if (paren_depth != 0)
						--paren_depth;
				}
				else if (ch == '{')
					++brace_depth;
				else if (ch == '}')
				{
					if (brace_depth != 0)
						--brace_depth;
				}
				else if ((ch == ';' || ch == '\n') && paren_depth == 0 && brace_depth == 0)
				{
					break;
				}

				++index;
			}

			StringView tail = input.substr(0, index);
			input.remove_prefix(index);
			return Strings::strip(tail);
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

	StackFrame::StackFrame(Entry* entry, StringView stream, StringView input, ExecuteFlags flags)
	{
		ConsoleState::instance().begin_execution(this);
		reset(entry, stream, input, flags);
	}

	StackFrame::~StackFrame()
	{
		ConsoleState::instance().end_execution(this);
	}

	void StackFrame::reset(Entry* new_entry, StringView new_stream, StringView new_input, ExecuteFlags new_flags)
	{
		entry  = new_entry;
		stream = new_stream;
		input  = new_input;
		result.clear();
		error.clear();
		status = ExecuteStatus::Success;
		flags  = new_flags;
		parameters.clear();
	}

	String StackFrame::fail(ExecuteStatus new_status, StringView message)
	{
		status = new_status;
		error  = String(message);
		result.clear();
		return error;
	}

	String StackFrame::succeed(StringView message)
	{
		status = ExecuteStatus::Success;
		error.clear();
		result = String(message);
		return result;
	}

	bool StackFrame::read_argument(StringView& out_argument)
	{
		auto& nested   = nested_execute_error();
		nested.active  = false;
		nested.status  = ExecuteStatus::Success;
		nested.message = {};

		stream = Strings::strip(stream);

		if (!stream.empty() && stream.front() == ',')
		{
			stream.remove_prefix(1);
			stream = Strings::strip(stream);
		}

		if (!stream.empty() && stream.front() == '%')
		{
			stream.remove_prefix(1);

			ExecuteResult execution = execute_view(stream, flags | ExecuteFlags::SingleCommand);
			if (!execution.success())
			{
				status         = execution.status;
				error          = execution.output;
				nested.active  = true;
				nested.status  = execution.status;
				nested.message = execution.output;
				return false;
			}

			char* copy = StackAllocator<char>::allocate(execution.output.size() + 1);
			strcpy(copy, execution.output.c_str());
			out_argument = StringView(copy, copy + execution.output.size());
			return true;
		}

		if (read_literal(stream, out_argument, ",;\n"))
			return true;

		if (nested.active)
		{
			status = nested.status;
			error  = nested.message;
		}

		return false;
	}

	bool StackFrame::has_more_args()
	{
		StringView copy = Strings::strip(stream);

		if (!copy.empty() && copy.front() == ',')
		{
			copy.remove_prefix(1);
			copy = Strings::strip(copy);
		}

		return !copy.empty();
	}

	StringView StackFrame::read_command_tail()
	{
		stream = Strings::strip(stream);

		if (stream.empty())
			return {};

		if (stream.front() == ',')
		{
			stream.remove_prefix(1);
			stream = Strings::strip(stream);
		}

		StringView tail = stream;
		stream.remove_prefix(stream.length());
		return Strings::strip(tail);
	}


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
		if (m_name == value)
			return *this;

		unregister_entry(this);
		m_name = String(value);
		register_entry(this);
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

	void VariableEntry::notify_variable_changed(StringView input)
	{
		VariableChangedEvent event{*this, input};

		for (Observer* observer : ConsoleState::instance().observers)
		{
			if (observer)
				observer->on_variable_changed(event);
		}
	}

	EntryType VariableEntry::type() const
	{
		return EntryType::Variable;
	}

	Command::Command(StringView name, String (*callback)(StackFrame* frame), const CommandOptionalArgs& args)
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
		auto& entries = ConsoleState::instance().entries;
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

		if (etl::find(ConsoleState::instance().observers.begin(), ConsoleState::instance().observers.end(), observer) ==
		    ConsoleState::instance().observers.end())
		{
			ConsoleState::instance().observers.push_back(observer);
		}
	}

	void remove_observer(Observer* observer)
	{
		auto& observers = ConsoleState::instance().observers;
		auto iter       = etl::find(observers.begin(), observers.end(), observer);
		if (iter != observers.end())
			observers.erase(iter);
	}

	void add_alias(StringView name, StringView expansion)
	{
		if (name.empty() || expansion.empty())
			return;
		ConsoleState::instance().aliases[String(name)] = String(expansion);
	}

	bool remove_alias(StringView name)
	{
		auto& map = ConsoleState::instance().aliases;
		auto iter = map.find(String(name));
		if (iter == map.end())
			return false;
		map.erase(iter);
		return true;
	}

	String find_alias(StringView name)
	{
		auto& map = ConsoleState::instance().aliases;
		auto iter = map.find(String(name));
		return iter == map.end() ? String() : iter->second;
	}

	Vector<String> aliases()
	{
		Vector<String> result;
		result.reserve(ConsoleState::instance().aliases.size());

		for (auto& [name, expansion] : ConsoleState::instance().aliases)
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
		for (auto& [name, expansion] : ConsoleState::instance().aliases)
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
		for (auto& [name, expansion] : ConsoleState::instance().aliases)
		{
			(void) expansion;
			if (prefix.empty() || StringView(name).starts_with(prefix))
				result.push_back(name);
		}
		etl::sort(result.begin(), result.end());
		return result;
	}

	String Command::execute(StackFrame* frame)
	{
		if (!m_callback)
			return frame->fail(ExecuteStatus::CommandHasNoCallback,
			                   Strings::format("Command '{}' has no callback", frame->entry->name()));

		frame->result.clear();
		frame->error.clear();
		frame->status = ExecuteStatus::Success;
		frame->parameters.clear();
		frame->parameters.reserve(m_parameters.size());

		for (const auto& parameter : m_parameters)
		{
			StringView raw_value;

			if (!frame->read_argument(raw_value))
			{
				if (parameter.has_default_value)
				{
					frame->parameters.push_back(parameter.default_value);
					continue;
				}

				return frame->fail(ExecuteStatus::MissingRequiredParameter,
				                   Strings::format("Expected parameter '{}: {}'", parameter.name, parameter.type));
			}

			Any parsed_value;
			if (parameter.parse == nullptr || !parameter.parse(raw_value, parsed_value))
			{
				return frame->fail(ExecuteStatus::ParameterParseFailed,
				                   Strings::format("Failed to parse parameter '{}: {}' from '{}'", parameter.name, parameter.type,
				                                   raw_value));
			}

			if (parameter.validate)
			{
				if (String validation = parameter.validate(parsed_value); !validation.empty())
				{
					return frame->fail(ExecuteStatus::ParameterValidationFailed,
					                   Strings::format("Parameter '{}': {}", parameter.name, validation));
				}
			}

			frame->parameters.push_back(std::move(parsed_value));
		}

		String result = m_callback(frame);

		if (frame->flags.all(ExecuteFlags::StrictParameters) && frame->has_more_args())
			return frame->fail(ExecuteStatus::TooManyParameters, Strings::format("Too many arguments for command '{}'", name()));

		if (frame->failed() && frame->status == ExecuteStatus::Success)
			frame->status = ExecuteStatus::CommandFailed;

		frame->result = result;
		return result;
	}

	ExecuteResult execute_view(StringView& stream, ExecuteFlags flags)
	{
		ExecutionReport report;
		Vector<String> results;

		auto fail_report = [&](ExecuteStatus status) {
			if (report.status == ExecuteStatus::Success)
				report.status = status;
		};

		skip_ignored_lines(stream);

		if (stream.empty())
			report.status = ExecuteStatus::EmptyInput;

		while (!stream.empty())
		{
			StringView statement_begin = stream;
			StringView cursor          = stream;
			StringView command;

			if (!read_identifier(cursor, command))
			{
				String error =
				        Strings::format("Unexpected token near '{}'", stream.substr(0, etl::min<usize>(stream.length(), 32)));
				results.push_back(error);
				fail_report(ExecuteStatus::UnexpectedToken);
				notify_command_executed(stream, {}, error, ExecuteStatus::UnexpectedToken);
				consume_statement_tail(stream);
				skip_ignored_lines(stream);
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
				stream = cursor;

				ExecutionReport alias_report = execute(alias, flags);
				if (!alias_report.output.empty())
					results.push_back(alias_report.output);

				if (!alias_report.success())
					fail_report(alias_report.status);

				notify_command_executed(exact_input, command, alias_report.output, alias_report.status);
				skip_ignored_lines(stream);
				continue;
			}

			Entry* entry = find(command);

			if (entry == nullptr)
			{
				finalize_input(cursor);
				stream = cursor;

				String error = Strings::format("Unknown console entry '{}'", command);
				notify_command_executed(exact_input, command, error, ExecuteStatus::UnknownEntry);
				fail_report(ExecuteStatus::UnknownEntry);

				if (!flags.all(ExecuteFlags::IgnoreUnknown))
					results.push_back(std::move(error));

				skip_ignored_lines(stream);
				continue;
			}

			if (!entry_visible(*entry))
			{
				finalize_input(cursor);
				stream = cursor;

				String error = Strings::format("Console entry '{}' is not available in current runtime policy", command);
				results.push_back(error);
				const ExecuteStatus status = execution_block_status(*entry);
				fail_report(status);
				notify_command_executed(exact_input, command, error, status);
				skip_ignored_lines(stream);
				continue;
			}

			String result;
			ExecuteStatus status = ExecuteStatus::Success;

			if (entry->type() == EntryType::Variable)
			{
				if (!statement.empty() && statement.front() == '(')
				{
					finalize_input(cursor);
					stream = cursor;
					result = Strings::format("'{}' is a console variable. Use '{}' or '{} = <value>'", command, command, command);
					status = ExecuteStatus::VariableCallSyntax;
				}
				else
				{
					if (String error = execution_block_reason(*entry); !error.empty())
					{
						finalize_input(cursor);
						stream = cursor;
						results.push_back(error);
						status = execution_block_status(*entry);
						fail_report(status);
						notify_command_executed(exact_input, command, error, status);
						skip_ignored_lines(stream);
						continue;
					}

					StackFrame frame(entry, statement, exact_input, flags);
					result = entry->execute(&frame);
					finalize_input(frame.stream);
					stream = frame.stream;
					status = frame.status;
				}
			}
			else
			{
				if (String error = execution_block_reason(*entry); !error.empty())
				{
					finalize_input(cursor);
					stream = cursor;
					results.push_back(error);
					status = execution_block_status(*entry);
					fail_report(status);
					notify_command_executed(exact_input, command, error, status);
					skip_ignored_lines(stream);
					continue;
				}

				if (statement.empty() || statement.front() != '(')
				{
					auto* typed_command = static_cast<Command*>(entry);

					if (!typed_command->can_invoke_without_parentheses())
					{
						finalize_input(cursor);
						stream = cursor;
						result = Strings::format("Command '{}' must be called as {}(...)", command, command);
						status = ExecuteStatus::CommandCallSyntax;
					}
					else
					{
						finalize_input(cursor);
						StackFrame frame(typed_command, {}, exact_input, flags);
						result = typed_command->execute(&frame);
						stream = cursor;
						status = frame.status;
					}
				}
				else
				{
					StringView arguments;
					cursor = statement;

					if (!read_parenthesized_contents(cursor, arguments))
					{
						result = Strings::format("Missing closing ')' for command '{}'", command);
						status = ExecuteStatus::MissingClosingParenthesis;
					}
					else
					{
						finalize_input(cursor);
						StackFrame frame(entry, arguments, exact_input, flags);
						result = entry->execute(&frame);
						stream = cursor;
						status = frame.status;
					}

					if (exact_input.empty())
						finalize_input(cursor);

					stream = cursor;
				}
			}

			if (exact_input.empty())
				finalize_input(stream);

			if (!result.empty())
				results.push_back(std::move(result));

			if (status != ExecuteStatus::Success)
				fail_report(status);

			notify_command_executed(exact_input, command, result, status);
			skip_ignored_lines(stream);

			if (flags.all(ExecuteFlags::SingleCommand))
				break;
		}

		report.output = Strings::join(results, "\n");
		return report;
	}

	ExecuteResult execute(StringView stream, ExecuteFlags flags)
	{
		return execute_view(stream, flags);
	}

	ExecuteResult execute(int argc, char** argv, ExecuteFlags flags)
	{
		if (argc <= 0 || argv == nullptr)
			return {.status = ExecuteStatus::EmptyInput};

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

		if (frame->read_argument(name))
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

			return frame->fail(ExecuteStatus::UnknownEntry, Strings::format("Unknown console entry '{}'", name));
		}

		Vector<String> lines = collect_entries([](const Entry&) { return true; });
		etl::sort(lines.begin(), lines.end());
		return Strings::join(lines, "\n");
	}

	trinex_static_console_command(console_list, .name = "list", .description = "List console entries", .usage = "list([prefix])")
	{
		StringView prefix;
		frame->read_argument(prefix);
		Vector<String> result = complete(prefix);

		if (result.empty())
			return prefix.empty() ? "No console entries registered"
			                      : Strings::format("No console entries for prefix '{}'", prefix);

		return Strings::join(result, "\n");
	}

	trinex_static_console_command(console_find, .name = "find", .description = "Search console entries", .usage = "find(<text>)")
	{
		StringView pattern_view;

		if (!frame->read_argument(pattern_view))
			return frame->fail(ExecuteStatus::MissingRequiredParameter, "Usage: find(<text>)");

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

		if (!frame->read_argument(name))
			return frame->fail(ExecuteStatus::MissingRequiredParameter, "Usage: reset(<variable>)");

		Entry* entry = find(name);

		if (entry == nullptr)
			return frame->fail(ExecuteStatus::UnknownEntry, Strings::format("Unknown console entry '{}'", name));

		if (entry->type() != EntryType::Variable)
			return frame->fail(ExecuteStatus::CommandFailed, Strings::format("'{}' is not a console variable", name));

		auto* variable = static_cast<VariableEntry*>(entry);
		if (variable->flags().all(Flags::ReadOnly))
			return frame->fail(ExecuteStatus::ReadOnly, Strings::format("'{}' is read-only", name));
		variable->reset(frame->input);
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

		if (!frame->read_argument(path))
			path = s_default_config_path;

		FileWriter writer(path, true);

		if (!writer.is_open())
			return frame->fail(ExecuteStatus::FileOpenFailed, Strings::format("Failed to open '{}' for writing", path));

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

		if (frame->read_argument(path))
		{
			StringView flag;
			if (frame->read_argument(flag))
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
			notify_config_loaded(path, result, ExecuteStatus::FileOpenFailed, true);
			return frame->fail(ExecuteStatus::FileOpenFailed, result);
		}

		ExecuteFlags flags = frame->flags;

		if (ignore_unknown)
		{
			flags = ExecuteFlags::IgnoreUnknown;
		}

		ExecuteResult result = execute(reader.read_string(), flags);
		notify_config_loaded(path, result.output, result.status, true);
		if (!result.success())
			frame->status = result.status;
		return result.output;
	}

	trinex_static_console_command(console_exec, .name = "exec", .description = "Execute console commands from file",
	                              .usage = "exec(<path>, [ignore_unknown=false])")
	{
		StringView path;
		bool ignore_unknown = false;

		if (!frame->read_argument(path))
			return frame->fail(ExecuteStatus::MissingRequiredParameter, "Usage: exec(<path>)");

		StringView flag;
		if (frame->read_argument(flag))
			Detail::parse_boolean(flag, ignore_unknown);

		FileReader reader(path);

		if (!reader.is_open())
		{
			String result = Strings::format("Failed to open '{}' for reading", path);
			notify_config_loaded(path, result, ExecuteStatus::FileOpenFailed, false);
			return frame->fail(ExecuteStatus::FileOpenFailed, result);
		}

		ExecuteFlags flags = frame->flags;

		if (ignore_unknown)
		{
			flags = ExecuteFlags::IgnoreUnknown;
		}

		ExecuteResult result = Console::execute(reader.read_string(), flags);
		notify_config_loaded(path, result.output, result.status, false);
		if (!result.success())
			frame->status = result.status;
		return result.output;
	}

	trinex_static_console_command(console_alias, .name = "alias", .description = "Register or replace alias",
	                              .usage = "alias(<name>, <expansion>)")
	{
		StringView name;
		if (!frame->read_argument(name))
			return frame->fail(ExecuteStatus::MissingRequiredParameter, "Usage: alias(<name>, <expansion>)");

		String expansion = String(frame->read_command_tail());
		if (expansion.empty())
			return frame->fail(ExecuteStatus::MissingRequiredParameter, "Usage: alias(<name>, <expansion>)");

		add_alias(name, expansion);
		return format_alias_line(name, expansion);
	}

	trinex_static_console_command(console_unalias, .name = "unalias", .description = "Remove alias", .usage = "unalias(<name>)")
	{
		StringView name;
		if (!frame->read_argument(name))
			return frame->fail(ExecuteStatus::MissingRequiredParameter, "Usage: unalias(<name>)");

		if (!remove_alias(name))
			return frame->fail(ExecuteStatus::UnknownEntry, Strings::format("Alias '{}' was not found", name));

		return Strings::format("Removed alias '{}'", name);
	}

	trinex_static_console_command(console_aliases, .name = "aliases", .description = "List aliases", .usage = "aliases()")
	{
		if (ConsoleState::instance().aliases.empty())
			return "No aliases registered";

		Vector<String> lines;
		lines.reserve(ConsoleState::instance().aliases.size());

		for (auto& [name, expansion] : ConsoleState::instance().aliases)
		{
			lines.push_back(format_alias_line(name, expansion));
		}

		etl::sort(lines.begin(), lines.end());
		return Strings::join(lines, "\n");
	}

}// namespace Trinex::Console
