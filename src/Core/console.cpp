#include <Core/console.hpp>
#include <Core/etl/algorithm.hpp>
#include <Core/etl/flat_set.hpp>
#include <Core/etl/utility.hpp>
#include <Core/etl/variant.hpp>
#include <Core/file_manager.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/string_functions.hpp>
#include <Core/types/path.hpp>

#include <any>
#include <peglib.h>

namespace Trinex::Console
{
	static constexpr const char* grammar = R"(
Program         <- Separator* StatementList? Separator* !.
StatementList   <- Statement (Separator+ Statement)*
Statement       <- Scope / Assignment / Invocation / Entry
Scope           <- EntryName '{' Separator* StatementList? Separator* '}'
Assignment      <- EntryName '=' Value
Invocation      <- EntryName '(' Newline* ArgumentList? Newline* ')'
Entry           <- EntryName
ArgumentList    <- Argument (Newline* ',' Newline* Argument)* (Newline* ',')?
Argument        <- Value
Value           <- Array / String / Boolean / Float / Integer / Bareword
Array           <- '[' Newline* ValueList? Newline* ']'
ValueList       <- Value (Newline* ',' Newline* Value)* (Newline* ',')?
Boolean         <- < ('true' / 'false') ![A-Za-z0-9_-] >
Float           <- < [+-]? ((([0-9]+ '.' [0-9]* / '.' [0-9]+) ([eE] [+-]? [0-9]+)?) / [0-9]+ [eE] [+-]? [0-9]+) >
Integer         <- HexInteger / BinaryInteger / DecimalInteger
HexInteger      <- < [+-]? '0x' [0-9a-fA-F]+ >
BinaryInteger   <- < [+-]? '0b' [01]+ >
DecimalInteger  <- < [+-]? [0-9]+ >
String          <- DoubleString / SingleString
DoubleString    <- < '"' (_Escape / [^"\\\r\n])* '"' >
SingleString    <- < "'" (_Escape / [^'\\\r\n])* "'" >
_Escape         <- '\\' [^\r\n]
Bareword        <- < [A-Za-z_][A-Za-z0-9_./:\\-]* >
EntryName       <- AbsoluteName / RelativeName
AbsoluteName    <- < '.' NameBody >
RelativeName    <- < NameBody >
NameBody        <- IdentifierRaw ('.' IdentifierRaw)*
Identifier      <- < IdentifierRaw >
IdentifierRaw   <- [A-Za-z_][A-Za-z0-9_-]*
Separator       <- ';' / Newline
Newline         <- '\r\n' / '\n' / '\r'
%whitespace     <- (_HSpace / _Comment)*
_HSpace         <- [ \t]
_Comment        <- '#'  (![\r\n] .)* / '//' (![\r\n] .)*
)";

	struct EnumInfo {
		u64* value;
		Refl::Enum* refl;
	};

	struct BareWord : public StringView {
		using StringView::StringView;

		BareWord(StringView view) : StringView(view) {}
	};

	struct Argument;

	struct ArgumentArray : Vector<Argument> {
		using Vector<Argument>::Vector;
	};

	struct Argument : public Variant<bool, i64, u64, f64, StringView, BareWord, ArgumentArray> {
		using Super = Variant<bool, i64, u64, f64, StringView, BareWord, ArgumentArray>;

		using Super::Super;
	};

	struct Statement;

	struct StatementList : Vector<Statement> {
		using Vector<Statement>::Vector;
	};

	struct Statement {
		enum class Type
		{
			Entry,
			Assignment,
			Invocation,
			Scope,
		};

		Type type = Type::Entry;
		StringView name;
		ArgumentArray args;
		StatementList statements;
	};

	class Interpreter
	{
	private:
		peg::parser m_parser;

	private:
		template<typename T>
		static const T& any_ref(const std::any& value)
		{
			const T* result = std::any_cast<T>(&value);
			if (result == nullptr)
			{
				throw std::runtime_error("Invalid console parser value");
			}

			return *result;
		}

		template<typename T>
		static const T* any_ptr(const std::any& value)
		{
			return std::any_cast<T>(&value);
		}

		template<typename T>
		static const T* first_ptr(const peg::SemanticValues& values)
		{
			for (const std::any& value : values)
			{
				if (const T* result = any_ptr<T>(value))
				{
					return result;
				}
			}

			return nullptr;
		}

		template<typename T>
		static const T& first_ref(const peg::SemanticValues& values)
		{
			for (const std::any& value : values)
			{
				if (const T* result = any_ptr<T>(value))
				{
					return *result;
				}
			}

			trinex_unreachable();
		}

		static StringView string_literal_value(StringView source)
		{
			if (source.length() < 2)
			{
				return "";
			}

			return source.substr(1, source.length() - 2);
		}

		static i64 parse_i64(StringView source)
		{
			String token(source);
			return static_cast<i64>(std::strtoll(token.c_str(), nullptr, 0));
		}

		static u64 parse_u64_binary(StringView source)
		{
			bool negative = false;
			usize index   = 0;

			if (!source.empty() && (source[0] == '-' || source[0] == '+'))
			{
				negative = source[0] == '-';
				index    = 1;
			}

			index += 2;

			u64 result = 0;
			for (; index < source.length(); ++index)
			{
				result = (result << 1) | static_cast<u64>(source[index] == '1');
			}

			return negative ? static_cast<u64>(-static_cast<i64>(result)) : result;
		}

		static Argument parse_integer(StringView source)
		{
			const bool negative = !source.empty() && source[0] == '-';
			const usize offset  = (!source.empty() && (source[0] == '-' || source[0] == '+')) ? 1 : 0;

			if (source.substr(offset).starts_with("0b"))
			{
				const u64 value = parse_u64_binary(source);
				return negative ? Argument(static_cast<i64>(value)) : Argument(value);
			}

			if (negative)
			{
				return Argument(parse_i64(source));
			}

			String token(source);
			return Argument(static_cast<u64>(std::strtoull(token.c_str(), nullptr, 0)));
		}

		static String resolve_name(StringView scope, StringView name)
		{
			if (name.starts_with("."))
			{
				return String(name.substr(1));
			}

			if (scope.empty())
			{
				return String(name);
			}

			return Strings::format("{}.{}", scope, name);
		}

		Entry* resolve_entry(StringView scope, StringView name, String* resolved_name = nullptr)
		{
			if (name.starts_with("."))
			{
				String candidate = String(name.substr(1));
				Entry* entry     = find(candidate);

				if (entry && resolved_name)
				{
					*resolved_name = etl::move(candidate);
				}

				return entry;
			}

			for (String current_scope(scope); !current_scope.empty();)
			{
				String candidate = Strings::format("{}.{}", current_scope, name);

				if (Entry* entry = find(candidate))
				{
					if (resolved_name)
					{
						*resolved_name = etl::move(candidate);
					}

					return entry;
				}

				const usize position = current_scope.rfind('.');

				if (position == String::npos)
				{
					break;
				}

				current_scope.resize(position);
			}

			String candidate = String(name);
			Entry* entry     = find(candidate);

			if (entry && resolved_name)
			{
				*resolved_name = etl::move(candidate);
			}

			return entry;
		}

	private:
		Interpreter() : m_parser(grammar) { configure_actions(); }

		void configure_actions()
		{
			m_parser["Boolean"] = [](const peg::SemanticValues& values) { return Argument(values.token() == "true"); };
			m_parser["Float"]   = [](const peg::SemanticValues& values) { return Argument(values.token_to_number<f64>()); };
			m_parser["Integer"] = [](const peg::SemanticValues& values) { return parse_integer(values.token()); };
			m_parser["String"] = [](const peg::SemanticValues& values) { return Argument(string_literal_value(values.token())); };
			m_parser["Bareword"] = [](const peg::SemanticValues& values) { return Argument(BareWord(values.token())); };

			m_parser["Array"] = [](const peg::SemanticValues& values) {
				if (auto result = first_ptr<ArgumentArray>(values))
				{
					return Argument(*result);
				}

				return Argument(ArgumentArray());
			};

			m_parser["ValueList"] = [](const peg::SemanticValues& values) {
				ArgumentArray array;
				array.reserve(values.size());

				for (const std::any& value : values)
				{
					if (const Argument* argument = any_ptr<Argument>(value))
					{
						array.emplace_back(*argument);
					}
				}

				return array;
			};

			m_parser["Value"]    = [](const peg::SemanticValues& values) { return first_ref<Argument>(values); };
			m_parser["Argument"] = [](const peg::SemanticValues& values) { return first_ref<Argument>(values); };

			m_parser["ArgumentList"] = [](const peg::SemanticValues& values) {
				ArgumentArray args;
				args.reserve(values.size());

				for (const std::any& value : values)
				{
					if (const Argument* argument = any_ptr<Argument>(value))
					{
						args.emplace_back(*argument);
					}
				}

				return args;
			};

			m_parser["Entry"] = [](const peg::SemanticValues& values) {
				return Statement{
				        .type = Statement::Type::Entry,
				        .name = first_ref<StringView>(values),
				};
			};

			m_parser["Assignment"] = [](const peg::SemanticValues& values) {
				Statement statement;
				statement.type = Statement::Type::Assignment;
				statement.name = first_ref<StringView>(values);
				statement.args.emplace_back(first_ref<Argument>(values));
				return statement;
			};

			m_parser["Invocation"] = [](const peg::SemanticValues& values) {
				Statement statement;
				statement.type = Statement::Type::Invocation;
				statement.name = first_ref<StringView>(values);

				if (auto result = first_ptr<ArgumentArray>(values))
				{
					statement.args = *result;
				}

				return statement;
			};

			m_parser["Scope"] = [](const peg::SemanticValues& values) {
				Statement statement;
				statement.type = Statement::Type::Scope;
				statement.name = first_ref<StringView>(values);

				for (const std::any& value : values)
				{
					if (const StatementList* statements = any_ptr<StatementList>(value))
					{
						statement.statements = *statements;
						break;
					}
				}

				return statement;
			};

			m_parser["Statement"] = [](const peg::SemanticValues& values) { return first_ref<Statement>(values); };

			m_parser["StatementList"] = [](const peg::SemanticValues& values) {
				StatementList statements;
				statements.reserve(values.size());

				for (const std::any& value : values)
				{
					if (const Statement* statement = any_ptr<Statement>(value))
					{
						statements.emplace_back(*statement);
					}
				}

				return statements;
			};

			m_parser["Program"] = [](const peg::SemanticValues& values) {
				if (values.empty())
				{
					return StatementList();
				}

				return first_ref<StatementList>(values);
			};

			m_parser["EntryName"]    = [](const peg::SemanticValues& values) { return first_ref<StringView>(values); };
			m_parser["AbsoluteName"] = [](const peg::SemanticValues& values) { return StringView(values.token()); };
			m_parser["RelativeName"] = [](const peg::SemanticValues& values) { return StringView(values.token()); };
			m_parser["Identifier"]   = [](const peg::SemanticValues& values) { return StringView(values.token()); };
		}

	public:
		static Interpreter* instance()
		{
			static thread_local Interpreter Interpreter;
			return &Interpreter;
		}

		ExecuteStatus parse(StatementList& statements, StringView source)
		{
			if (source.empty())
			{
				return ExecuteStatus::EmptyInput;
			}

			StatementList result;
			const bool success = m_parser.parse(source, result);

			if (!success)
			{
				return ExecuteStatus::UnexpectedToken;
			}

			statements = etl::move(result);
			return ExecuteStatus::Success;
		}

		ExecuteStatus execute(Entry* entry, const ExecuteContext& ctx, bool assignment)
		{
			if (entry == nullptr)
			{
				return ctx.flags.any(ExecuteFlags::IgnoreUnknown) ? ExecuteStatus::Success : ExecuteStatus::UnknownEntry;
			}

			if (entry->flags().any(EntryFlags::Hidden))
			{
				return ExecuteStatus::EntryUnavailable;
			}

			if (assignment && entry->flags().any(EntryFlags::ReadOnly))
			{
				return ExecuteStatus::ReadOnly;
			}

			if (entry->type() == EntryType::Variable && !assignment && !ctx.args.empty())
			{
				return ExecuteStatus::VariableCallSyntax;
			}

			if (entry->type() == EntryType::Command && assignment)
			{
				return ExecuteStatus::CommandCallSyntax;
			}

			return entry->execute(ctx);
		}

		ExecuteStatus execute(StringView scope, const Statement& statement, String* output, ExecuteFlags flags)
		{
			String fullname = resolve_name(scope, statement.name);

			if (statement.type == Statement::Type::Scope)
			{
				for (const Statement& child : statement.statements)
				{
					ExecuteStatus status = execute(fullname, child, output, flags);

					if (status != ExecuteStatus::Success)
					{
						return status;
					}
				}

				return ExecuteStatus::Success;
			}

			ExecuteContext ctx{
			        .args   = Span<Argument>(const_cast<Argument*>(statement.args.data()), statement.args.size()),
			        .output = output,
			        .flags  = flags,
			};

			return execute(resolve_entry(scope, statement.name, &fullname), ctx, statement.type == Statement::Type::Assignment);
		}

		ExecuteStatus execute(StringView source, String* output, ExecuteFlags flags)
		{
			StatementList statements;
			ExecuteStatus status = parse(statements, source);

			if (status != ExecuteStatus::Success)
			{
				return status;
			}

			for (const Statement& statement : statements)
			{
				status = execute("", statement, output, flags);

				if (status != ExecuteStatus::Success)
				{
					return status;
				}
			}

			return ExecuteStatus::Success;
		}
	};

	class Manager
	{
	private:
		struct Compare {
			bool operator()(const Entry* first, const Entry* second) const { return first->name() < second->name(); }
		};

		FlatSet<Entry*, Compare> m_entries;

	public:
		Manager& bind(Entry* entry)
		{
			trinex_assert(find(entry->name()) == nullptr);
			const bool inserted = m_entries.insert(entry).second;
			trinex_assert(inserted);
			return *this;
		}

		Manager& unbind(Entry* entry)
		{
			trinex_assert(find(entry->name()) == entry);
			m_entries.erase(entry);
			return *this;
		}

		Entry* find(StringView name) const
		{
			auto it = etl::lower_bound(m_entries.begin(), m_entries.end(), name,
			                           [](const Entry* entry, StringView name) { return entry->name() < name; });

			if (it == m_entries.end())
				return nullptr;

			Entry* entry = *it;

			if (entry->name() != name)
				return nullptr;

			return entry;
		}

		usize find(StringView name, const FunctionRef<void(Entry*)>& action) const
		{
			auto it = etl::lower_bound(m_entries.begin(), m_entries.end(), name,
			                           [](const Entry* entry, StringView name) { return entry->name() < name; });

			usize count = 0;

			for (; it != m_entries.end(); ++it)
			{
				Entry* entry = *it;

				if (!entry->name().starts_with(name))
					return count;

				action(entry);
				++count;
			}

			return count;
		}

		static Manager* instance()
		{
			static Manager manager;
			return &manager;
		}
	};

	template<typename T>
	concept ArgumentString = etl::same_as<T, StringView> || etl::same_as<T, BareWord>;

	template<typename Dst, typename Src>
	static inline ExecuteStatus (*s_type_convertor)(Dst*, const Src*) =
	        [](Dst*, const Src*) -> ExecuteStatus { return ExecuteStatus::ValueParseFailed; };

	template<typename T>
	static inline ExecuteStatus (*s_type_convertor<T, T>)(T*, const T*) = [](T* dst, const T* src) -> ExecuteStatus {
		*dst = *src;
		return ExecuteStatus::Success;
	};

	template<etl::arithmetic Dst, etl::arithmetic Src>
	static inline ExecuteStatus (*s_type_convertor<Dst, Src>)(Dst*, const Src*) = [](Dst* dst, const Src* src) -> ExecuteStatus {
		*dst = static_cast<Dst>(*src);
		return ExecuteStatus::Success;
	};

	template<etl::arithmetic Dst, ArgumentString Src>
	static inline ExecuteStatus (*s_type_convertor<Dst, Src>)(Dst*, const Src*) = [](Dst* dst, const Src* src) -> ExecuteStatus {
		if constexpr (etl::same_as<Dst, bool>)
		{
			if (Strings::boolean_of(*src, *dst))
				return ExecuteStatus::Success;

			return ExecuteStatus::ValueParseFailed;
		}
		else if constexpr (etl::unsigned_integral<Dst>)
		{
			if (Strings::unsigned_of(*src, *dst))
				return ExecuteStatus::Success;

			return ExecuteStatus::ValueParseFailed;
		}
		else if constexpr (etl::signed_integral<Dst>)
		{
			if (Strings::signed_of(*src, *dst))
				return ExecuteStatus::Success;

			return ExecuteStatus::ValueParseFailed;
		}
		else if constexpr (etl::floating_point<Dst>)
		{
			if (Strings::floating_of(*src, *dst))
				return ExecuteStatus::Success;

			return ExecuteStatus::ValueParseFailed;
		}

		return ExecuteStatus::ValueParseFailed;
	};

	template<ArgumentString Src>
	static inline ExecuteStatus (*s_type_convertor<EnumInfo, Src>)(EnumInfo*, const Src*) =
	        [](EnumInfo* dst, const Src* src) -> ExecuteStatus {
		StringView view = *src;

		auto& entries = dst->refl->entries();

		auto it = etl::find_if(entries.begin(), entries.end(),
		                       [&view](const Refl::Enum::Entry& entry) -> bool { return entry.name == view; });

		if (it == entries.end())
			return ExecuteStatus::ValueParseFailed;

		(*dst->value) = it->value;
		return ExecuteStatus::Success;
	};

	template<ArgumentString Src>
	static inline ExecuteStatus (*s_type_convertor<String, Src>)(String*, const Src*) =
	        [](String* dst, const Src* src) -> ExecuteStatus {
		*dst = *src;
		return ExecuteStatus::Success;
	};

	template<etl::arithmetic Src>
	static inline ExecuteStatus (*s_type_convertor<String, Src>)(String*, const Src*) =
	        [](String* dst, const Src* src) -> ExecuteStatus {
		if constexpr (etl::same_as<Src, bool>)
		{
			(*dst) = (*src) ? "true" : "false";
			return ExecuteStatus::Success;
		}
		else
		{
			(*dst) = Strings::format("{}", *src);
			return ExecuteStatus::Success;
		}

		trinex_unreachable();
	};


	template<typename T>
	static ExecuteStatus store_internal(T* dst, const Argument* src)
	{
		auto visitor = [dst]<typename Src>(const Src& value) -> ExecuteStatus { return s_type_convertor<T, Src>(dst, &value); };
		return etl::visit(visitor, *src);
	}

	ExecuteStatus Entry::store(bool* dst, const Argument* src)
	{
		return store_internal(dst, src);
	}

	ExecuteStatus Entry::store(u64* dst, const Argument* src)
	{
		return store_internal(dst, src);
	}

	ExecuteStatus Entry::store(i64* dst, const Argument* src)
	{
		return store_internal(dst, src);
	}

	ExecuteStatus Entry::store(f64* dst, const Argument* src)
	{
		return store_internal(dst, src);
	}

	ExecuteStatus Entry::store(String* dst, const Argument* src)
	{
		return store_internal(dst, src);
	}

	ExecuteStatus Entry::store(ArrayInterface* dst, const Argument* src)
	{
		dst->clear();

		if (const ArgumentArray* array = etl::get_if<ArgumentArray>(src))
		{
			for (const Argument& argument : *array)
			{
				ExecuteStatus status = dst->append(&argument);

				if (status != ExecuteStatus::Success)
					return status;
			}
		}
		else
		{
			return dst->append(src);
		}

		return ExecuteStatus::Success;
	}

	ExecuteStatus Entry::store_enum(u64* dst, const Argument* src, Refl::Enum* refl)
	{
		EnumInfo result = {
		        .value = dst,
		        .refl  = refl,
		};

		return store_internal(&result, src);
	}

	ExecuteStatus Entry::store_bitfield(u64* dst, const Argument* src, Refl::Enum* refl)
	{
		if (const ArgumentArray* array = etl::get_if<ArgumentArray>(src))
		{
			*dst = 0;

			for (const Argument& argument : *array)
			{
				u64 tmp              = 0;
				ExecuteStatus status = store_bitfield(&tmp, &argument, refl);

				if (status != ExecuteStatus::Success)
					return status;

				*dst |= tmp;
			}
		}
		else
		{
			return store_enum(dst, src, refl);
		}

		return ExecuteStatus::Success;
	}

	ExecuteStatus Entry::store(String* dst, bool* src)
	{
		(*dst) = (*src) ? "true" : "false";
		return ExecuteStatus::Success;
	}

	ExecuteStatus Entry::store(String* dst, u64* src)
	{
		(*dst) = Strings::format("{}", *src);
		return ExecuteStatus::Success;
	}

	ExecuteStatus Entry::store(String* dst, i64* src)
	{
		(*dst) = Strings::format("{}", *src);
		return ExecuteStatus::Success;
	}

	ExecuteStatus Entry::store(String* dst, f64* src)
	{
		(*dst) = Strings::format("{}", *src);
		return ExecuteStatus::Success;
	}

	ExecuteStatus Entry::store(String* dst, String* src)
	{
		(*dst) = *src;
		return ExecuteStatus::Success;
	}

	ExecuteStatus Entry::store(String* dst, ArrayInterface* src)
	{
		(*dst) = Strings::format("<array:{}>", src->size());
		return ExecuteStatus::Success;
	}

	EntryType VariableEntry::type() const
	{
		return EntryType::Variable;
	}

	Command::Command(StringView name, Callback callback, StringView description, EntryFlags flags)
	    : Entry(name, description, flags), m_callback(etl::move(callback))
	{}

	EntryType Command::type() const
	{
		return EntryType::Command;
	}

	ExecuteStatus Command::execute(const ExecuteContext& ctx)
	{
		if (!m_callback)
		{
			return ExecuteStatus::CommandHasNoCallback;
		}

		return m_callback(ctx);
	}

	Entry::Entry(StringView name, StringView description, EntryFlags flags)
	    : m_name(name), m_description(description), m_flags(flags)
	{
		Manager::instance()->bind(this);
	}

	Entry::~Entry()
	{
		Manager::instance()->unbind(this);
	}

	ENGINE_EXPORT Entry* find(StringView name)
	{
		return Manager::instance()->find(name);
	}

	ENGINE_EXPORT usize find(StringView name, const FunctionRef<void(Entry*)>& action)
	{
		return Manager::instance()->find(name, action);
	}

	ENGINE_EXPORT ExecuteStatus execute(StringView source, String* output, ExecuteFlags flags)
	{
		return Interpreter::instance()->execute(source, output, flags);
	}

	ENGINE_EXPORT ExecuteStatus execute_config(const Path& path)
	{
		FileReader reader(Path("[configs]:") / path);

		if (!reader.is_open())
		{
			trinex_warning(Log::Engine, "Failed to load config '%s'", path.c_str());
			return ExecuteStatus::FileOpenFailed;
		}

		const Console::ExecuteStatus status = Console::execute(reader.read_string());

		if (status != Console::ExecuteStatus::Success)
		{
			trinex_warning(Log::Engine, "Failed to execute config '%s' with status %u", path.c_str(),
			               static_cast<u32>(status.value));
		}
		return status;
	}
}// namespace Trinex::Console
