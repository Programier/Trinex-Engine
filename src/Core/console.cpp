#include <Core/blob.hpp>
#include <Core/console.hpp>
#include <Core/etl/span.hpp>
#include <Core/etl/utility.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/reflection/new/function.hpp>
#include <Core/reflection/new/object.hpp>
#include <Core/reflection/new/pointer_type.hpp>
#include <Core/reflection/new/property.hpp>
#include <Core/reflection/new/scope.hpp>
#include <Core/reflection/new/type.hpp>
#include <Core/types/path.hpp>

#include <any>
#include <peglib.h>

namespace Trinex::Console
{
	static constexpr const char* grammar = R"(
Program <- Separator* StatementList? Separator* !.
StatementList <- Statement (Separator+ Statement)*

Statement <- Scope / Assignment / Expression

Scope <- Context '{' Separator* StatementList? Separator* '}'
Assignment <- Context '=' RawValue
Expression <- Context

Context <- Absolute? Operation Postfix*
Absolute <- < '.' >
Operation <- IdentifierRaw Arguments?
Postfix <- Member / Arguments
Member <- '.' Operation

Arguments <- '(' ArgumentList? ')'
ArgumentList <- Argument (',' Argument)* ','?
Argument <- < (Quoted / Balanced / !(',' / ')') .)+ >

RawValue <- < (Quoted / Balanced / !(';' / '}' / Newline) .)+ >

Balanced <- '(' (Quoted / Balanced / !')' .)* ')'
          / '[' (Quoted / Balanced / !']' .)* ']'
          / '{' (Quoted / Balanced / !'}' .)* '}'

Quoted <- '"' ('\\' . / !'"' .)* '"'
        / "'" ('\\' . / !"'" .)* "'"

IdentifierRaw <- < [A-Za-z_][A-Za-z0-9_-]* >

Separator <- ';' / Newline
Newline <- '\r\n' / '\n' / '\r'

%whitespace <- (_HSpace / _Comment)*
_HSpace <- [ \t]
_Comment <- '#' (![\r\n] .)* / '//' (![\r\n] .)*
)";

	using Statements = Array<struct Statement>;
	using Context    = Array<struct Operation>;
	using Arguments  = Array<StringView>;

	struct Operation {
		StringView name;
		Arguments arguments;
	};

	struct Statement {
		enum class Kind
		{
			Scope,
			Assignment,
			Expression
		};

		Kind kind = Kind::Expression;

		Context context;
		StringView value;
		Statements children;
	};

	class Parser
	{
	private:
		peg::parser m_parser;

		void bind_actions()
		{
			using SV = peg::SemanticValues;

			m_parser["IdentifierRaw"] = [](const SV& vs) { return StringView(vs.token()); };
			m_parser["Argument"]      = [](const SV& vs) { return Strings::trim(vs.token()); };

			m_parser["ArgumentList"] = [](const SV& vs) {
				Arguments result;

				for (const auto& value : vs)
				{
					result.emplace_back(std::any_cast<StringView>(value));
				}

				return result;
			};

			m_parser["Arguments"] = [](const SV& vs) {
				if (vs.empty())
					return Arguments{};

				return std::any_cast<Arguments>(vs[0]);
			};

			m_parser["Operation"] = [](const SV& vs) {
				Operation result;
				for (const auto& value : vs)
				{
					if (const auto* name = std::any_cast<StringView>(&value))
						result.name = *name;
					else if (const auto* arguments = std::any_cast<Arguments>(&value))
					{
						result.arguments = *arguments;
					}
				}
				return result;
			};

			m_parser["Member"]  = [](const SV& vs) { return std::any_cast<Operation>(vs[0]); };
			m_parser["Postfix"] = [](const SV& vs) {
				if (const auto* operation = std::any_cast<Operation>(&vs[0]))
					return *operation;

				Operation result;
				result.arguments = std::any_cast<Arguments>(vs[0]);
				return result;
			};

			m_parser["Context"] = [](const SV& vs) {
				Context result;

				for (const auto& value : vs)
				{
					if (const auto* operation = std::any_cast<Operation>(&value))
					{
						if (operation->name.empty())
						{
							if (result.empty())
								return Context{};

							result.back().arguments = operation->arguments;
						}
						else
						{
							result.emplace_back(*operation);
						}
					}
				}

				return result;
			};

			// RawValue -> StringView
			m_parser["RawValue"] = [](const SV& vs) { return Strings::trim(vs.token()); };

			// Assignment -> Statement
			m_parser["Assignment"] = [](const SV& vs) {
				Statement result;

				result.kind    = Statement::Kind::Assignment;
				result.context = std::any_cast<Context>(vs[0]);
				result.value   = std::any_cast<StringView>(vs[1]);

				return result;
			};

			// Expression -> Statement
			m_parser["Expression"] = [](const SV& vs) {
				Statement result;

				result.kind    = Statement::Kind::Expression;
				result.context = std::any_cast<Context>(vs[0]);

				return result;
			};

			// Scope -> Statement
			m_parser["Scope"] = [](const SV& vs) {
				Statement result;

				result.kind    = Statement::Kind::Scope;
				result.context = std::any_cast<Context>(vs[0]);

				if (vs.size() > 1)
				{
					result.children = std::any_cast<Statements>(vs[1]);
				}

				return result;
			};

			// Statement -> Statement
			m_parser["Statement"] = [](const SV& vs) { return std::any_cast<Statement>(vs[0]); };

			// StatementList -> Statements
			m_parser["StatementList"] = [](const SV& vs) {
				Statements result;

				for (const auto& value : vs)
				{
					result.emplace_back(std::any_cast<Statement>(value));
				}

				return result;
			};

			// Program -> Statements
			m_parser["Program"] = [](const SV& vs) {
				if (vs.empty())
					return Statements{};

				return std::any_cast<Statements>(vs[0]);
			};
		}

	public:
		Parser() : m_parser(grammar) { bind_actions(); }

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

	static constexpr u64 console_flags_shift = 48;

	struct ExecutionContext {
		void* object                   = nullptr;
		Reflection::Object* reflection = nullptr;
		ExecutionFlags flags           = ExecutionFlags::Undefined;

		inline bool is_valid() const
		{
			if (reflection)
			{
				if (reflection->is<Reflection::Scope>())
				{
					return true;
				}

				return object != nullptr;
			}

			return false;
		}
	};

	class BoundProperty final : public Reflection::Property
	{
	private:
		void* m_address;

	public:
		BoundProperty(Name name, void* address, Reflection::Type* type, EntryFlags flags, Reflection::Object* owner)
		    : Property(name, type,
		               (static_cast<u64>(flags) << console_flags_shift) |
		                       (flags.any(EntryFlags::ReadOnly) ? Reflection::Property::IsReadOnly : 0),
		               owner),
		      m_address(address)
		{}

		void* address(void*) override { return is_read_only() ? nullptr : m_address; }
		const void* load(const void*, void* = nullptr) override { return m_address; }
		void store(void*, void* value) override
		{
			if (!is_read_only())
				type()->copy_assign(m_address, value);
		}
	};

	class BoundFunction final : public Reflection::Object
	{
	private:
		Ref<Reflection::Function> m_function;

	public:
		BoundFunction(Name name, Reflection::Function* function, EntryFlags flags, Reflection::Object* owner)
		    : Object(name, static_cast<u64>(flags) << console_flags_shift, owner), m_function(retain_ref(function))
		{}

		Object* resolve(const Reflection::Class* base) override
		{
			if (Object* object = Object::resolve(base))
				return object;

			return m_function->resolve(base);
		}
	};

	static Reflection::Scope* root()
	{
		static Reflection::Scope scope("Console");
		return &scope;
	}

	static Reflection::Scope* scope_of(StringView path, bool create)
	{
		Reflection::Scope* scope = root();

		for (usize begin = 0; scope != nullptr && begin < path.size();)
		{
			const usize end = path.find('.', begin);
			const Name name(path.substr(begin, end - begin));
			if (!name.is_valid())
				return nullptr;

			if (Reflection::Object* object = scope->find(name))
			{
				scope = object->resolve<Reflection::Scope>();
			}
			else if (create)
			{
				scope = Ref<Reflection::Scope>::make(name, Reflection::Object::None, scope).value();
			}
			else
			{
				return nullptr;
			}

			begin = end == StringView::npos ? path.size() : end + 1;
		}

		return scope;
	}

	static Reflection::Object* find_binding(StringView name)
	{
		const usize position     = name.rfind('.');
		Reflection::Scope* scope = scope_of(position == StringView::npos ? "" : name.substr(0, position), false);
		return scope ? scope->find(Name(name.substr(position == StringView::npos ? 0 : position + 1))) : nullptr;
	}


	static DynamicValue value_of(Reflection::Type* type, const void* value)
	{
		DynamicValue result(type);
		return result && value != nullptr && type->copy_assign(result.data(), value) ? result : DynamicValue();
	}

	static DynamicValue execute_function(const ExecutionContext& context, Reflection::Function* function,
	                                     Span<const StringView> arguments)
	{
		const auto& parameters = function->args();
		{
			usize required_arguments = function->args().size();

			if (context.object != nullptr)
			{
				for (const Ref<Reflection::Property>& prop : function->args())
				{
					if (prop->is_this())
					{
						--required_arguments;
					}
				}
			}

			if (arguments.size() != required_arguments)
			{
				trinex_warning(Log::Console, "Invalid number of function arguments");
				return {};
			}
		}

		const usize alignment = function->alignment() == 0 ? alignof(void*) : function->alignment();
		void* storage         = ByteAllocator::allocate_aligned(function->size() == 0 ? 1 : function->size(), alignment);
		Array<Reflection::Property*> constructed;
		bool success = true;

		for (usize index = 0, argument_index = 0; success && index < parameters.size(); ++index)
		{
			Reflection::Property* parameter = parameters[index].value();
			void* value                     = parameter->address(storage);
			success                         = value != nullptr && parameter->type()->construct(value);

			if (!success)
				break;

			constructed.emplace_back(parameter);

			if (parameter->is_this())
			{
				success = parameter->type()->copy_assign(value, &context.object);
			}
			else
			{
				const StringView argument = arguments[argument_index++];
				success                   = parameter->type()->parse(value, argument) == argument.size();
			}
		}

		DynamicValue result;

		if (success)
		{
			function->invoke(storage);
			if (Reflection::Property* return_value = function->result())
				result = value_of(const_cast<Reflection::Type*>(return_value->type()), return_value->load(storage));
		}
		else
		{
			trinex_warning(Log::Console, "Failed to prepare function arguments");
		}

		while (!constructed.empty())
		{
			Reflection::Property* parameter = constructed.back();
			parameter->type()->destruct(parameter->address(storage));
			constructed.pop_back();
		}

		ByteAllocator::deallocate(static_cast<unsigned char*>(storage));
		return result;
	}

	template<typename F>
	static DynamicValue execute_operations(ExecutionContext context, Span<const Operation> operations, F&& func)
	{
		if (!context.is_valid())
			return {};

		Array<DynamicValue> result;
		result.reserve(operations.size());

		while (!operations.empty())
		{
			const Operation* operation = operations.data();
			operations                 = operations.subspan(1);

			if (Reflection::PointerType* pointer = context.reflection->cast<Reflection::PointerType>())
			{
				context.object     = context.object ? *static_cast<void**>(context.object) : nullptr;
				context.reflection = pointer->type();
			}

			if (context.reflection == nullptr || (context.object == nullptr && !context.reflection->is<Reflection::Scope>()))
				return {};

			Reflection::Object* entry = context.reflection->find(operation->name);

			if (entry == nullptr)
				return {};

			if (Reflection::Function* function = entry->resolve<Reflection::Function>())
			{
				DynamicValue& value = result.emplace_back(execute_function(context, function, operation->arguments));

				if (function->result() == nullptr)
				{
					if (!operations.empty())
						return {};
					return func(context);
				}

				if (!value)
					return {};

				context.object     = value.data();
				context.reflection = value.type();
			}
			else if (Reflection::Property* property = entry->resolve<Reflection::Property>())
			{
				context.object     = property->address(context.object);
				context.reflection = const_cast<Reflection::Type*>(property->type());

				if (context.object == nullptr)
					return {};
			}
			else if (Reflection::Scope* child = entry->resolve<Reflection::Scope>())
			{
				context.object     = nullptr;
				context.reflection = child;
			}
			else
			{
				return {};
			}
		}

		return func(context);
	}

	static DynamicValue execute_statement(ExecutionContext context, const Statement& statement)
	{
		return execute_operations(context, statement.context, [&statement](const ExecutionContext& context) -> DynamicValue {
			if (!context.is_valid())
			{
				if (!context.flags.any(ExecutionFlags::IgnoreUnknown))
					trinex_warning(Log::Console, "Failed to resolve console context");
				return {};
			}

			if (statement.kind == Statement::Kind::Scope)
			{
				if (!context.reflection->is<Reflection::Scope>())
				{
					trinex_warning(Log::Console, "Console context is not a scope");
					return {};
				}

				DynamicValue result;
				for (const Statement& child : statement.children) result = execute_statement(context, child);
				return result;
			}

			Reflection::Type* type = context.reflection->resolve<Reflection::Type>();
			if (type == nullptr || context.object == nullptr)
				return {};

			if (statement.kind == Statement::Kind::Assignment)
			{
				DynamicValue value(type);
				if (!value || type->parse(value.data(), statement.value) != statement.value.size() ||
				    !type->copy_assign(context.object, value.data()))
				{
					trinex_warning(Log::Console, "Failed to assign console value");
					return {};
				}
			}

			return value_of(type, context.object);
		});
	}

	trinex_api void bind(StringView name, void* address, Reflection::Type* reflection, StringView description, EntryFlags flags)
	{
		const usize position     = name.rfind('.');
		Reflection::Scope* scope = scope_of(position == StringView::npos ? "" : name.substr(0, position), true);
		const Name binding_name(name.substr(position == StringView::npos ? 0 : position + 1));
		trinex_verify(address != nullptr && reflection != nullptr && scope != nullptr && binding_name.is_valid() &&
		              find_binding(name) == nullptr);
		(void) Ref<BoundProperty>::make(binding_name, address, reflection, flags, scope);
		(void) description;
	}

	trinex_api void bind(StringView name, Reflection::Function* function, StringView description, EntryFlags flags)
	{
		const usize position     = name.rfind('.');
		Reflection::Scope* scope = scope_of(position == StringView::npos ? "" : name.substr(0, position), true);
		const Name binding_name(name.substr(position == StringView::npos ? 0 : position + 1));
		trinex_verify(function != nullptr && scope != nullptr && binding_name.is_valid() && find_binding(name) == nullptr);
		(void) Ref<BoundFunction>::make(binding_name, function, flags, scope);
		(void) description;
	}

	trinex_api void bind(StringView name, void (*function)(void*), Reflection::Type* ret, Reflection::Type** args, usize count,
	                     StringView description, EntryFlags flags)
	{
		// trinex_verify(function != nullptr && !name.empty() && find_binding(name) == nullptr);
	}

	trinex_api void unbind(StringView name)
	{
		if (Reflection::Object* binding = find_binding(name))
			binding->owner(nullptr);
	}

	trinex_api DynamicValue execute(StringView source, ExecutionFlags flags)
	{
		static thread_local Parser parser;

		Statements statements;

		if (parser.parse(source, statements))
		{
			DynamicValue result;

			ExecutionContext context = {nullptr, root(), flags};

			for (const Statement& statement : statements)
			{
				result = execute_statement(context, statement);
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
