#include <Core/console.hpp>
#include <Core/etl/algorithm.hpp>
#include <Core/etl/charconv.hpp>
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
			u128 stack_location;
			u64 execution_depth = 0;

			static ConsoleState& instance()
			{
				static ConsoleState state;
				return state;
			}

			inline void begin_execution()
			{
				if (execution_depth++ == 0)
				{
					stack_location = StackByteAllocator::location();
				}
			}

			inline void end_execution()
			{
				if (--execution_depth == 0)
				{
					StackByteAllocator::location(stack_location);
				}
			}
		};

		struct ConsoleExecutionScope {
			ConsoleExecutionScope() { ConsoleState::instance().begin_execution(); }
			~ConsoleExecutionScope() { ConsoleState::instance().end_execution(); }
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

		static void notify_command_executed(StringView command, const String& output, ExecuteStatus status)
		{
			CommandExecutedEvent event{command, output, status};

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

		static bool is_horizontal_ws(char c)
		{
			return c == ' ' || c == '\t' || c == '\r';
		}

		static bool is_newline(char c)
		{
			return c == '\n';
		}

		static bool is_symbol(char c)
		{
			switch (c)
			{
				case '(':
				case ')':
				case '=':
				case ',':
				case ';':
				case '%': return true;

				default: return false;
			}
		}

		static StringView view_of(StringView source, std::size_t from, std::size_t to)
		{
			return StringView{source.data() + from, to - from};
		}

		static void advance_location(SourceLocation& location, char c)
		{
			++location.offset;

			if (is_newline(c))
			{
				++location.line;
				location.column = 1;
			}
			else
			{
				++location.column;
			}
		}

		static void append_token(Token*& head, Token*& tail, TokenKind kind, StringView text, SourceLocation begin,
		                         SourceLocation end)
		{
			if (text.size() == 0)
			{
				return;
			}

			Token* node      = trx_stack_new Token;
			node->kind       = kind;
			node->identifier = text;
			node->begin      = begin;
			node->end        = end;

			if (tail)
			{
				node->prev = tail;
				tail->next = node;
				tail       = node;
			}
			else
			{
				head = tail = node;
			}
		}

		static Token* tokenize(StringView source, Token*& head, Token*& tail)
		{
			const u32 n = source.size();
			u32 i       = 0;

			SourceLocation location;

			bool at_line_start = true;

			auto consume = [&]() -> char {
				char c = source[i];
				++i;
				advance_location(location, c);
				return c;
			};

			while (i < n)
			{
				char c = source[i];

				// Ignore full-line comments and section-like lines.
				if (at_line_start)
				{
					u32 j = i;

					while (j < n && is_horizontal_ws(source[j]))
					{
						++j;
					}

					if (j < n)
					{
						const bool ignored_line =
						        source[j] == '#' || source[j] == '[' || (source[j] == '/' && j + 1 < n && source[j + 1] == '/');

						if (ignored_line)
						{
							while (i < n && !is_newline(source[i]))
							{
								consume();
							}

							if (i < n && is_newline(source[i]))
							{
								consume();
							}

							at_line_start = true;
							continue;
						}
					}
				}

				// Skip spaces/tabs/CR. Do not create tokens for them.
				if (is_horizontal_ws(c))
				{
					consume();
					continue;
				}

				// Skip newline. Do not create token for it.
				if (is_newline(c))
				{
					consume();
					at_line_start = true;
					continue;
				}

				// Quoted literal.
				if (c == '"' || c == '\'')
				{
					const u32 start_index               = i;
					const SourceLocation start_location = location;

					const char quote = consume();// opening quote

					while (i < n)
					{
						char x = source[i];

						if (x == '\\')
						{
							consume();

							if (i < n)
							{
								consume();
							}

							continue;
						}

						if (x == quote)
						{
							consume();// closing quote
							break;
						}

						consume();
					}

					append_token(head, tail, TokenKind::StringLiteral, view_of(source, start_index, i), start_location, location);

					at_line_start = false;
					continue;
				}

				// One-character symbols.
				if (is_symbol(c))
				{
					const u32 start_index               = i;
					const SourceLocation start_location = location;

					consume();

					TokenKind kind = TokenKind::BareLiteral;

					switch (c)
					{
						case '(': kind = TokenKind::LParen; break;
						case ')': kind = TokenKind::RParen; break;
						case '=': kind = TokenKind::Assign; break;
						case ',': kind = TokenKind::Comma; break;
						case ';': kind = TokenKind::Semicolon; break;
						case '%': kind = TokenKind::NestedExecute; break;
						default: break;
					}

					append_token(head, tail, kind, view_of(source, start_index, i), start_location, location);

					at_line_start = false;
					continue;
				}

				// Regular token: identifier or bare literal chunk.
				const u32 start_index               = i;
				const SourceLocation start_location = location;

				while (i < n)
				{
					char x = source[i];

					if (is_horizontal_ws(x) || is_newline(x) || is_symbol(x) || x == '"' || x == '\'')
					{
						break;
					}

					consume();
				}

				StringView text       = view_of(source, start_index, i);
				const bool identifier = (std::isalpha(static_cast<unsigned char>(text.front())) || text.front() == '_') &&
				                        etl::all_of(text.begin() + 1, text.end(), [](char ch) {
					                        return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
				                        });
				append_token(head, tail, identifier ? TokenKind::Identifier : TokenKind::BareLiteral, text, start_location,
				             location);

				at_line_start = false;
			}

			return head;
		}

		static Token* tokenize(StringView source)
		{
			Token* head = nullptr;
			Token* tail = nullptr;
			return tokenize(source, head, tail);
		}

		static Token* tokenize(u32 argc, char** argv)
		{
			Token* head = nullptr;
			Token* tail = nullptr;

			for (u32 i = 0; i < argc; ++i)
			{
				tokenize(argv[i], head, tail);
			}

			return head;
		}

		static bool is_identifier_token(StringView input)
		{
			if (input.empty())
				return false;

			const char first = input.front();

			if (!(std::isalpha(static_cast<unsigned char>(first)) || first == '_'))
				return false;

			for (usize index = 1; index < input.length(); ++index)
			{
				const char ch = input[index];
				if (!(std::isalnum(static_cast<unsigned char>(ch)) || ch == '_'))
					return false;
			}

			return true;
		}

		static Token* skip_statement_separators(Token* token)
		{
			while (token != nullptr && token->kind == TokenKind::Semicolon)
			{
				token = token->next;
			}

			return token;
		}

		static bool separated_by_newline(const Token* left, const Token* right)
		{
			return left != nullptr && right != nullptr && left->end.line < right->begin.line;
		}

		static Token* find_statement_end(Token* begin, Token* limit)
		{
			if (begin == nullptr)
				return limit;

			Token* previous   = begin;
			Token* cursor     = begin;
			usize paren_depth = 0;

			while (cursor != limit)
			{
				if (cursor->kind == TokenKind::LParen)
				{
					++paren_depth;
				}
				else if (cursor->kind == TokenKind::RParen)
				{
					if (paren_depth == 0)
						break;

					--paren_depth;
				}

				if (paren_depth == 0)
				{
					if (cursor->kind == TokenKind::Semicolon)
						break;

					if (cursor != begin && separated_by_newline(previous, cursor))
						break;
				}

				previous = cursor;
				cursor   = cursor->next;
			}

			return cursor;
		}

		static Token* find_matching_paren(Token* open_token, Token* limit)
		{
			if (open_token == nullptr || open_token->kind != TokenKind::LParen)
				return nullptr;

			usize depth = 1;

			for (Token* cursor = open_token->next; cursor != limit; cursor = cursor->next)
			{
				if (cursor->kind == TokenKind::LParen)
				{
					++depth;
				}
				else if (cursor->kind == TokenKind::RParen)
				{
					--depth;

					if (depth == 0)
						return cursor;
				}
			}

			return nullptr;
		}

		static bool should_insert_space(StringView left, StringView right)
		{
			if (left.empty() || right.empty())
				return false;

			if (right == ")" || right == "," || right == ";")
				return false;

			if (left == "(" || left == "%" || left == ",")
				return false;

			if (right == "(")
				return false;

			if (left == "=" || right == "=")
				return true;

			return true;
		}

		static String tokens_to_string(Token* begin, Token* end = nullptr)
		{
			String result;
			Token* previous = nullptr;

			for (Token* cursor = begin; cursor != end; cursor = cursor->next)
			{
				if (previous != nullptr)
				{
					if (separated_by_newline(previous, cursor))
					{
						result.push_back('\n');
					}
					else if (should_insert_space(previous->identifier, cursor->identifier))
					{
						result.push_back(' ');
					}
				}

				result += cursor->identifier;
				previous = cursor;
			}

			return result;
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
		bool parse_boolean(StringView value, bool& out)
		{
			return Strings::boolean_of(value, out);
		}

		bool parse_signed(StringView value, i64& out)
		{
			return Strings::signed_of(value, out);
		}

		bool parse_unsigned(StringView value, u64& out)
		{
			return Strings::unsigned_of(value, out);
		}

		bool parse_floating(StringView value, f64& out)
		{
			return Strings::floating_of(value, out);
		}

		bool parse_string(StringView value, StringView& out)
		{
			if (value.empty())
			{
				out = "";
				return false;
			}

			if (value.starts_with("\"'"))
			{
				if (value.size() == 1 || value.front() != value.back())
				{
					out = "";
					return false;
				}

				value.remove_prefix(1);
				value.remove_suffix(1);
				out = value;
				return true;
			}

			out = value;
			return true;
		}

		ENGINE_EXPORT bool parse_string(StringView value, String& out)
		{
			StringView tmp;
			if (parse_string(value, tmp))
			{
				out = tmp;
				return true;
			}
			return false;
		}

		bool parse_reflected_enum(StringView value, Refl::Enum* reflection, u64& out)
		{
			if (reflection == nullptr)
				return false;

			if (value.empty())
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

			if (const auto* entry = find_entry(value))
			{
				out = static_cast<u64>(entry->value);
				return true;
			}

			return false;
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

		String format_reflected_enum(Refl::Enum* reflection, u64 value)
		{
			if (reflection == nullptr)
				return {};

			if (const auto* entry = reflection->entry(static_cast<EnumerateType>(value)))
				return entry->name.to_string();

			if (!reflection->is_bit_flags())
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

		String format_parse_error(StringView name, StringView value)
		{
			return Strings::format("Failed to parse value '{}' for '{}'", value, name);
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

	static ExecuteResult execute_tokens(Token*& stream, Token* limit, ExecuteFlags flags);

	StackFrame::StackFrame(Entry* entry, Token* stream, Token* input, Token* end, ExecuteFlags flags)
	{
		reset(entry, stream, input, end, flags);
	}

	StackFrame& StackFrame::reset(Entry* new_entry, Token* new_stream, Token* new_input, Token* new_end, ExecuteFlags new_flags)
	{
		entry  = new_entry;
		input  = new_input;
		stream = new_stream;
		end    = new_end;
		result.clear();
		error.clear();
		status = ExecuteStatus::Success;
		flags  = new_flags;
		parameters.clear();

		return *this;
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

	bool StackFrame::read_argument(Token*& out)
	{
		auto& nested   = nested_execute_error();
		nested.active  = false;
		nested.status  = ExecuteStatus::Success;
		nested.message = {};

		while (stream != nullptr && stream != end && stream->kind == TokenKind::Comma)
		{
			stream = stream->next;
		}

		if (stream != nullptr && stream != end && stream->kind == TokenKind::NestedExecute)
		{
			stream                  = stream->next;
			ExecuteResult execution = execute_tokens(stream, end, flags | ExecuteFlags::SingleCommand);
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
			Token* token      = trx_stack_new Token;
			token->kind       = TokenKind::BareLiteral;
			token->identifier = StringView(copy, copy + execution.output.size());
			out               = token;
			return true;
		}

		if (stream != nullptr && stream != end)
		{
			out    = stream;
			stream = stream->next;
			return true;
		}

		if (nested.active)
		{
			status = nested.status;
			error  = nested.message;
		}

		return false;
	}

	bool StackFrame::read_argument(StringView& out)
	{
		Token* token = nullptr;

		if (!read_argument(token))
			return false;

		out = token->identifier;
		return true;
	}

	bool StackFrame::has_more_args()
	{
		Token* cursor = stream;

		while (cursor != nullptr && cursor != end && cursor->kind == TokenKind::Comma)
		{
			cursor = cursor->next;
		}

		return cursor != nullptr && cursor != end;
	}

	Token* StackFrame::read_command_tail()
	{
		while (stream != nullptr && stream != end && stream->kind == TokenKind::Comma)
		{
			stream = stream->next;
		}

		if (stream == nullptr || stream == end)
			return {};

		Token* result = stream;
		stream        = end;
		return result;
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

	void VariableEntry::notify_variable_changed()
	{
		VariableChangedEvent event{*this, ""};

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
			Token* token = nullptr;

			if (!frame->read_argument(token))
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
			if (parameter.parse == nullptr || !parameter.parse(token->identifier, parsed_value))
			{
				return frame->fail(ExecuteStatus::ParameterParseFailed,
				                   Strings::format("Failed to parse parameter '{}: {}' from '{}'", parameter.name, parameter.type,
				                                   token->identifier));
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

	static ExecuteResult execute_tokens(Token*& stream, Token* limit, ExecuteFlags flags)
	{
		ExecutionReport report;
		Vector<String> results;

		auto fail_report = [&](ExecuteStatus status) {
			if (report.status == ExecuteStatus::Success)
				report.status = status;
		};

		stream = skip_statement_separators(stream);

		if (stream == nullptr || stream == limit)
			report.status = ExecuteStatus::EmptyInput;

		while (stream != nullptr && stream != limit)
		{
			stream                 = skip_statement_separators(stream);
			Token* statement_begin = stream;

			if (statement_begin == nullptr || statement_begin == limit)
				break;

			if (!is_identifier_token(statement_begin->identifier))
			{
				Token* statement_end = find_statement_end(statement_begin, limit);
				String error         = Strings::format("Unexpected token near '{}'", statement_begin->identifier);
				results.push_back(error);
				fail_report(ExecuteStatus::UnexpectedToken);
				notify_command_executed({}, error, ExecuteStatus::UnexpectedToken);
				stream = statement_end;
				continue;
			}

			StringView command = statement_begin->identifier;
			Token* after_name  = statement_begin->next;
			const bool same_line =
			        after_name != nullptr && after_name != limit && !separated_by_newline(statement_begin, after_name);
			const bool has_parens    = same_line && after_name->kind == TokenKind::LParen;
			const bool has_assign    = same_line && after_name->kind == TokenKind::Assign;
			auto plain_statement_end = [&]() -> Token* {
				if (after_name == nullptr || after_name == limit)
					return after_name;

				if (separated_by_newline(statement_begin, after_name))
					return after_name;

				return find_statement_end(after_name, limit);
			};

			if (String alias = find_alias(command); !alias.empty() && !has_parens && !has_assign)
			{
				stream = after_name;

				ExecutionReport alias_report = execute(alias, flags);
				if (!alias_report.output.empty())
					results.push_back(alias_report.output);

				if (!alias_report.success())
					fail_report(alias_report.status);

				notify_command_executed(command, alias_report.output, alias_report.status);
				continue;
			}

			Entry* entry = find(command);

			if (entry == nullptr)
			{
				Token* statement_end = plain_statement_end();
				String error         = Strings::format("Unknown console entry '{}'", command);
				notify_command_executed(command, error, ExecuteStatus::UnknownEntry);
				fail_report(ExecuteStatus::UnknownEntry);

				if (!flags.all(ExecuteFlags::IgnoreUnknown))
					results.push_back(std::move(error));

				stream = statement_end;
				continue;
			}

			if (!entry_visible(*entry))
			{
				Token* statement_end = has_parens ? find_matching_paren(after_name, limit) : plain_statement_end();
				statement_end        = statement_end == nullptr ? find_statement_end(statement_begin, limit)
				                                                : (has_parens ? statement_end->next : statement_end);
				String error         = Strings::format("Console entry '{}' is not available in current runtime policy", command);
				results.push_back(error);
				const ExecuteStatus status = execution_block_status(*entry);
				fail_report(status);
				notify_command_executed(command, error, status);
				stream = statement_end;
				continue;
			}

			String result;
			ExecuteStatus status = ExecuteStatus::Success;

			if (entry->type() == EntryType::Variable)
			{
				Token* statement_end = plain_statement_end();

				if (has_parens)
				{
					Token* close = find_matching_paren(after_name, limit);
					stream       = close == nullptr ? statement_end : close->next;
					result = Strings::format("'{}' is a console variable. Use '{}' or '{} = <value>'", command, command, command);
					status = ExecuteStatus::VariableCallSyntax;
				}
				else
				{
					if (String error = execution_block_reason(*entry); !error.empty())
					{
						stream = statement_end;
						results.push_back(error);
						status = execution_block_status(*entry);
						fail_report(status);
						notify_command_executed(command, error, status);
						continue;
					}

					StackFrame frame(entry, after_name, statement_begin, statement_end, flags);
					result = entry->execute(&frame);
					stream = frame.stream;
					status = frame.status;
				}
			}
			else
			{
				if (String error = execution_block_reason(*entry); !error.empty())
				{
					Token* statement_end = has_parens ? find_matching_paren(after_name, limit) : plain_statement_end();
					statement_end        = statement_end == nullptr ? find_statement_end(statement_begin, limit)
					                                                : (has_parens ? statement_end->next : statement_end);
					stream               = statement_end;
					results.push_back(error);
					status = execution_block_status(*entry);
					fail_report(status);
					notify_command_executed(command, error, status);
					continue;
				}

				if (!has_parens)
				{
					auto* typed_command = static_cast<Command*>(entry);

					if (!typed_command->can_invoke_without_parentheses())
					{
						stream = after_name;
						result = Strings::format("Command '{}' must be called as {}(...)", command, command);
						status = ExecuteStatus::CommandCallSyntax;
					}
					else
					{
						StackFrame frame(typed_command, after_name, statement_begin, after_name, flags);
						result = typed_command->execute(&frame);
						stream = after_name;
						status = frame.status;
					}
				}
				else
				{
					Token* close = find_matching_paren(after_name, limit);

					if (close == nullptr)
					{
						stream = find_statement_end(statement_begin, limit);
						result = Strings::format("Missing closing ')' for command '{}'", command);
						status = ExecuteStatus::MissingClosingParenthesis;
					}
					else
					{
						StackFrame frame(entry, after_name->next, statement_begin, close, flags);
						result = entry->execute(&frame);
						stream = close->next;
						status = frame.status;
					}
				}
			}

			if (!result.empty())
				results.push_back(std::move(result));

			if (status != ExecuteStatus::Success)
				fail_report(status);

			notify_command_executed(command, result, status);

			if (flags.all(ExecuteFlags::SingleCommand))
				break;
		}

		report.output = Strings::join(results, "\n");
		return report;
	}

	ExecuteResult execute_view(StringView& stream, ExecuteFlags flags)
	{
		ConsoleExecutionScope scope;

		Token* head          = tokenize(stream);
		Token* cursor        = head;
		ExecuteResult result = execute_tokens(cursor, nullptr, flags);

		if (cursor == nullptr)
		{
			stream = {};
		}
		else
		{
			stream.remove_prefix(cursor->begin.offset);
		}

		return result;
	}

	ExecuteResult execute(StringView stream, ExecuteFlags flags)
	{
		return execute_view(stream, flags);
	}

	ExecuteResult execute(int argc, char** argv, ExecuteFlags flags)
	{
		ConsoleExecutionScope scope;

		Token* head   = tokenize(argc, argv);
		Token* cursor = head;
		return execute_tokens(cursor, nullptr, flags);
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
		variable->reset();
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
			variable->reset();
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

		String expansion = tokens_to_string(frame->read_command_tail(), frame->end);

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
