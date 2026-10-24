#include <Core/blob.hpp>
#include <Core/console.hpp>
#include <Core/etl/allocator.hpp>
#include <Core/etl/flat_set.hpp>
#include <Core/etl/span.hpp>
#include <Core/etl/utility.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/reflection/new/function.hpp>
#include <Core/reflection/new/object.hpp>
#include <Core/reflection/new/type.hpp>
#include <Core/types/path.hpp>

#include <any>
#include <peglib.h>

namespace Trinex::Console
{
	static constexpr const char* grammar = R"(
Program <- Separator* StatementList? Separator* !.
StatementList <- Statement (Separator+ Statement)*
Statement <- Scope / Assignment / Invocation / Entry
Scope <- EntryName '{' Separator* StatementList? Separator* '}'
Assignment <- EntryName '=' Value
Invocation <- EntryName '(' Newline* ArgumentList? Newline* ')'
Entry <- EntryName
ArgumentList <- Argument (Newline* ',' Newline* Argument)* (Newline* ',')?
Argument <- Value
Value <- Call / Reference / Literal
Call <- EntryName '(' Newline* ArgumentList? Newline* ')'
Reference <- < ('.'? NameBody) >
Literal <- < (Array / Object / Group / String / Scalar) >
Array <- '[' Newline* ValueList? Newline* ']'
ValueList <- Value (Newline* ',' Newline* Value)* (Newline* ',')?
Object <- '{' ObjectElement* '}'
ObjectElement <- Array / Object / Group / String / ![{}] .
Group <- '(' GroupElement* ')'
GroupElement <- Array / Object / Group / String / ![()] .
String <- DoubleString / SingleString
DoubleString <- '"' (_Escape / [^"\\\r\n])* '"'
SingleString <- "'" (_Escape / [^'\\\r\n])* "'"
_Escape <- '\\' [^\r\n]
Scalar <- (!(',' / ')' / ';' / Newline) .)+
EntryName <- AbsoluteName / RelativeName
AbsoluteName <- < '.' NameBody >
RelativeName <- < NameBody >
NameBody <- IdentifierRaw ('.' IdentifierRaw)*
IdentifierRaw <- [A-Za-z_][A-Za-z0-9_-]*
Separator <- ';' / Newline
Newline <- '\r\n' / '\n' / '\r'
%whitespace <- (_HSpace / _Comment)*
_HSpace <- [ \t]
_Comment <- '#' (![\r\n] .)* / '//' (![\r\n] .)*
)";

	struct Argument;

	struct Arguments : Array<Argument> {
		using Array<Argument>::Array;
	};

	struct Argument {
		enum class Kind : u8
		{
			Literal,
			Reference,
			Call,
		};

		Kind kind = Kind::Literal;
		StringView value;
		Arguments arguments;
	};

	struct Statement;
	using Statements = Array<Statement>;

	struct Statement {
		enum class Type
		{
			Entry,
			Assignment,
			Invocation,
			Scope
		};

		Type type = Type::Entry;
		StringView name;
		Arguments arguments;
		Statements statements;
	};

	class Parser
	{
	private:
		peg::parser m_parser;

		template<typename T>
		static const T* any_ptr(const std::any& value)
		{
			return std::any_cast<T>(&value);
		}

		template<typename T>
		static const T& first_ref(const peg::SemanticValues& values)
		{
			for (const std::any& value : values)
			{
				if (const T* result = any_ptr<T>(value))
					return *result;
			}

			trinex_unreachable();
		}

		void configure_actions()
		{
			m_parser["Value"]    = [](const peg::SemanticValues& values) { return first_ref<Argument>(values); };
			m_parser["Argument"] = [](const peg::SemanticValues& values) { return first_ref<Argument>(values); };
			m_parser["Literal"]  = [](const peg::SemanticValues& values) {
				return Argument{Argument::Kind::Literal, values.token()};
			};
			m_parser["Reference"] = [](const peg::SemanticValues& values) {
				return Argument{Argument::Kind::Reference, values.token()};
			};
			m_parser["ArgumentList"] = [](const peg::SemanticValues& values) {
				Arguments arguments;
				for (const std::any& value : values)
					if (const Argument* argument = any_ptr<Argument>(value))
						arguments.emplace_back(*argument);
				return arguments;
			};
			m_parser["Call"] = [](const peg::SemanticValues& values) {
				Argument argument{.kind = Argument::Kind::Call, .value = first_ref<StringView>(values)};
				for (const std::any& value : values)
					if (const Arguments* arguments = any_ptr<Arguments>(value))
						argument.arguments = *arguments;
				return argument;
			};
			m_parser["Entry"] = [](const peg::SemanticValues& values) {
				return Statement{.type = Statement::Type::Entry, .name = first_ref<StringView>(values)};
			};
			m_parser["Assignment"] = [](const peg::SemanticValues& values) {
				Statement statement{.type = Statement::Type::Assignment, .name = first_ref<StringView>(values)};
				statement.arguments.emplace_back(first_ref<Argument>(values));
				return statement;
			};
			m_parser["Invocation"] = [](const peg::SemanticValues& values) {
				Statement statement{.type = Statement::Type::Invocation, .name = first_ref<StringView>(values)};
				for (const std::any& value : values)
					if (const Arguments* arguments = any_ptr<Arguments>(value))
						statement.arguments = *arguments;
				return statement;
			};
			m_parser["Scope"] = [](const peg::SemanticValues& values) {
				Statement statement{.type = Statement::Type::Scope, .name = first_ref<StringView>(values)};
				for (const std::any& value : values)
					if (const Statements* statements = any_ptr<Statements>(value))
						statement.statements = *statements;
				return statement;
			};
			m_parser["Statement"]     = [](const peg::SemanticValues& values) { return first_ref<Statement>(values); };
			m_parser["StatementList"] = [](const peg::SemanticValues& values) {
				Statements statements;
				for (const std::any& value : values)
					if (const Statement* statement = any_ptr<Statement>(value))
						statements.emplace_back(*statement);
				return statements;
			};
			m_parser["Program"] = [](const peg::SemanticValues& values) {
				return values.empty() ? Statements() : first_ref<Statements>(values);
			};
			m_parser["EntryName"]    = [](const peg::SemanticValues& values) { return first_ref<StringView>(values); };
			m_parser["AbsoluteName"] = [](const peg::SemanticValues& values) { return StringView(values.token()); };
			m_parser["RelativeName"] = [](const peg::SemanticValues& values) { return StringView(values.token()); };
		}

	public:
		Parser() : m_parser(grammar) { configure_actions(); }

		bool parse(StringView source, Statements& statements)
		{
			if (source.empty())
			{
				trinex_warning(Log::Console, "Empty console input");
				return false;
			}

			if (!m_parser.parse(source, statements))
			{
				trinex_warning(Log::Console, "Failed to parse console input: %.*s", static_cast<int>(source.size()),
				               source.data());
				return {};
			}

			return true;
		}
	};

	struct Binding {
		String name;
		String description;
		EntryFlags flags               = EntryFlags::Undefined;
		void* address                  = nullptr;
		Reflection::Object* reflection = nullptr;
	};

	struct BindingCompare {
		using is_transparent = void;

		bool operator()(const Binding& lhs, const Binding& rhs) const { return lhs.name < rhs.name; }
		bool operator()(const Binding& lhs, StringView rhs) const { return lhs.name < rhs; }
		bool operator()(StringView lhs, const Binding& rhs) const { return lhs < rhs.name; }
	};

	static FlatSet<Binding, BindingCompare>& bindings()
	{
		static FlatSet<Binding, BindingCompare> result;
		return result;
	}

	static const Binding* find_binding(StringView name)
	{
		auto it = bindings().find(name);
		return it == bindings().end() ? nullptr : &*it;
	}

	static String scope_name(StringView scope, StringView name)
	{
		if (name.starts_with('.'))
			return String(name.substr(1));

		String result(scope);
		if (!result.empty())
			result += '.';
		result += name;
		return result;
	}

	static const Binding* resolve_binding(StringView scope, StringView name)
	{
		if (name.starts_with('.'))
			return find_binding(name.substr(1));

		for (String current_scope(scope); !current_scope.empty();)
		{
			String candidate = current_scope;
			candidate += '.';
			candidate += name;

			if (const Binding* binding = find_binding(candidate))
				return binding;

			const usize position = current_scope.rfind('.');
			if (position == String::npos)
				break;

			current_scope.resize(position);
		}

		return find_binding(name);
	}

	static bool store(void* dst, const Reflection::Type* type, const Argument& argument)
	{
		if (dst != nullptr && type != nullptr && type->parse(dst, argument.value) == argument.value.size())
			return true;

		trinex_warning(Log::Console, "Failed to parse '%.*s' as '%s'", static_cast<int>(argument.value.size()),
		               argument.value.data(), type ? type->name().to_string().c_str() : "<null>");
		return false;
	}

	static DynamicValue value_of(Reflection::Type* type, const void* value)
	{
		DynamicValue result(type);

		if (value != nullptr && result && type->copy_assign(result.data(), value))
			return result;

		trinex_warning(Log::Console, "Failed to create dynamic console value");
		return {};
	}

	static DynamicValue resolve_value(StringView scope, const Argument& argument, Reflection::Type* expected_type);

	static DynamicValue execute_function(StringView scope, const Reflection::Function* function, Span<const Argument> arguments)
	{
		const auto& parameters = function->args();

		if (arguments.size() != parameters.size())
		{
			trinex_warning(Log::Console, "Command expects %zu arguments", parameters.size());
			return {};
		}

		void* context = ByteAllocator::allocate_aligned(function->size() == 0 ? 1 : function->size(), function->alignment());
		Array<Reflection::Property*> constructed;

		auto construct = [&constructed, context](Reflection::Property* property) {
			void* value = property->address(context);
			return value != nullptr && property->type()->construct(value) && (constructed.emplace_back(property), true);
		};

		bool success = !function->result() || construct(function->result());

		for (usize index = 0; success && index < parameters.size(); ++index)
		{
			Reflection::Property* parameter = parameters[index].value();
			DynamicValue value = resolve_value(scope, arguments[index], const_cast<Reflection::Type*>(parameter->type()));
			success            = value && value.type() == parameter->type() && construct(parameter) &&
			                     parameter->type()->copy_assign(parameter->address(context), value.data());
		}

		if (success)
		{
			function->invoke(context);
			DynamicValue result;
			if (function->result())
				result = value_of(const_cast<Reflection::Type*>(function->result()->type()), function->result()->load(context));

			while (!constructed.empty())
			{
				Reflection::Property* property = constructed.back();
				property->type()->destruct(property->address(context));
				constructed.pop_back();
			}

			ByteAllocator::deallocate(static_cast<unsigned char*>(context));
			return result;
		}
		else
		{
			trinex_warning(Log::Console, "Failed to prepare command arguments");
		}

		while (!constructed.empty())
		{
			Reflection::Property* property = constructed.back();
			property->type()->destruct(property->address(context));
			constructed.pop_back();
		}

		ByteAllocator::deallocate(static_cast<unsigned char*>(context));
		return {};
	}

	static DynamicValue resolve_value(StringView scope, const Argument& argument, Reflection::Type* expected_type)
	{
		if (argument.kind == Argument::Kind::Literal)
		{
			DynamicValue result(expected_type);
			return result && store(result.data(), expected_type, argument) ? result : DynamicValue();
		}

		const Binding* binding = resolve_binding(scope, argument.value);

		if (binding == nullptr)
		{
			if (argument.kind == Argument::Kind::Reference)
			{
				DynamicValue result(expected_type);
				return result && store(result.data(), expected_type, argument) ? result : DynamicValue();
			}

			trinex_warning(Log::Console, "Unknown command '%.*s'", static_cast<int>(argument.value.size()),
			               argument.value.data());
			return {};
		}

		if (argument.kind == Argument::Kind::Call)
		{
			if (Reflection::Function* function = binding->reflection->cast<Reflection::Function>())
			{
				return execute_function(scope, function, argument.arguments);
			}

			trinex_warning(Log::Console, "Binding '%s' is not a command", binding->name.c_str());
			return {};
		}

		if (Reflection::Type* type = binding->reflection->cast<Reflection::Type>())
			return value_of(type, binding->address);

		trinex_warning(Log::Console, "Binding '%s' is not a value", binding->name.c_str());
		return {};
	}

	static DynamicValue execute_statement(StringView scope, StringView name, Span<const Argument> arguments, ExecuteFlags flags,
	                                      Statement::Type type)
	{
		const bool assignment  = type == Statement::Type::Assignment;
		const Binding* binding = resolve_binding(scope, name);

		if (binding == nullptr)
		{
			if (!flags.any(ExecuteFlags::IgnoreUnknown))
				trinex_warning(Log::Console, "Unknown console binding '%.*s'", static_cast<int>(name.size()), name.data());
			return {};
		}

		if (binding->flags.any(EntryFlags::Hidden))
		{
			trinex_warning(Log::Console, "Console binding '%s' is unavailable", binding->name.c_str());
			return {};
		}

		if (assignment && binding->flags.any(EntryFlags::ReadOnly))
		{
			trinex_warning(Log::Console, "Console binding '%s' is read-only", binding->name.c_str());
			return {};
		}

		if (Reflection::Function* function = binding->reflection->cast<Reflection::Function>())
		{
			if (assignment)
			{
				trinex_warning(Log::Console, "Command '%s' cannot be assigned", binding->name.c_str());
				return {};
			}

			return execute_function(scope, function, arguments);
		}

		Reflection::Type* value_type = binding->reflection->cast<Reflection::Type>();

		if (value_type == nullptr)
		{
			trinex_warning(Log::Console, "Binding '%s' has unsupported reflection", binding->name.c_str());
			return {};
		}

		if (arguments.empty())
		{
			return value_of(value_type, binding->address);
		}

		if (!assignment || arguments.size() != 1)
		{
			trinex_warning(Log::Console, "Variable '%s' requires assignment with one value", binding->name.c_str());
			return {};
		}

		DynamicValue value = resolve_value(scope, arguments[0], value_type);

		if (!value || value.type() != value_type || !value_type->copy_assign(binding->address, value.data()))
		{
			trinex_warning(Log::Console, "Failed to assign '%s'", binding->name.c_str());
			return {};
		}

		return value_of(value_type, binding->address);
	}

	static DynamicValue execute(StringView scope, const Statement& statement, ExecuteFlags flags)
	{
		if (statement.type == Statement::Type::Scope)
		{
			const String child_scope = scope_name(scope, statement.name);
			DynamicValue result;

			for (const Statement& child : statement.statements)
			{
				result = execute(child_scope, child, flags);
			}

			return result;
		}

		return execute_statement(scope, statement.name, statement.arguments, flags, statement.type);
	}

	trinex_api void bind(StringView name, void* address, Reflection::Type* reflection, StringView description, EntryFlags flags)
	{
		trinex_verify(address != nullptr && reflection != nullptr && !name.empty() && find_binding(name) == nullptr);
		bindings().insert({String(name), String(description), flags, address, reflection});
	}

	trinex_api void bind(StringView name, Reflection::Function* function, StringView description, EntryFlags flags)
	{
		trinex_verify(function != nullptr && !name.empty() && find_binding(name) == nullptr);
		bindings().insert({String(name), String(description), flags, nullptr, function});
	}

	trinex_api void bind(StringView name, void (*function)(void*), Reflection::Type* ret, Reflection::Type** args, usize count,
	                     StringView description, EntryFlags flags)
	{
		trinex_verify(function != nullptr && !name.empty() && find_binding(name) == nullptr);
	}

	trinex_api void unbind(StringView name)
	{
		if (auto it = bindings().find(name); it != bindings().end())
			bindings().erase(it);
	}

	trinex_api DynamicValue execute(StringView source, ExecuteFlags flags)
	{
		static thread_local Parser parser;

		Statements statements;

		if (parser.parse(source, statements))
		{
			DynamicValue result;

			for (const Statement& statement : statements)
			{
				result = execute("", statement, flags);
			}

			return result;
		}

		return {};
	}

	trinex_api DynamicValue execute_config(const Path& path)
	{
		if (auto data = rootfs()->map(Path("[configs]:") / path))
			return Console::execute(data->as<StringView>());

		trinex_warning(Log::Console, "Failed to load config '%s'", path.c_str());

		return DynamicValue();
	}
}// namespace Trinex::Console
