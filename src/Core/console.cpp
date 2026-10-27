#include <Core/blob.hpp>
#include <Core/console.hpp>
#include <Core/etl/span.hpp>
#include <Core/etl/utility.hpp>
#include <Core/etl/variant.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Core/reflection/const_type.hpp>
#include <Core/reflection/indirection_type.hpp>
#include <Core/reflection/native_function.hpp>
#include <Core/reflection/object.hpp>
#include <Core/reflection/property.hpp>
#include <Core/reflection/scope.hpp>
#include <Core/reflection/type.hpp>
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
Assignment <- Context '=' AssignmentValue
Expression <- Context

Context <- Absolute? Operation Postfix*
Absolute <- < '.' >
Operation <- IdentifierRaw Arguments?
Postfix <- Member / Arguments
Member <- '.' Operation

Arguments <- '(' ArgumentList? ')'
ArgumentList <- Argument (',' Argument)* ','?
Argument <- ContextArgument / ArgumentValue
AssignmentValue <- ContextArgument / RawValue
ContextArgument <- '@' Context
ArgumentValue <- < (Quoted / Balanced / !(',' / ')') .)+ >

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
	using Argument   = Variant<StringView, Context>;
	using Arguments  = Array<Argument>;

	struct Operation
	{
		StringView name;
		Arguments arguments;
	};

	struct Statement
	{
		enum class Kind
		{
			Scope,
			Assignment,
			Expression
		};

		Kind kind = Kind::Expression;

		Context context;
		Argument value;
		Statements children;
	};

	class Parser
	{
	private:
		peg::parser m_parser;

		void bind_actions()
		{
			using SV = peg::SemanticValues;

			m_parser["IdentifierRaw"]   = [](const SV& vs) { return StringView(vs.token()); };
			m_parser["ArgumentValue"]   = [](const SV& vs) { return Argument(Strings::trim(vs.token())); };
			m_parser["RawValue"]        = [](const SV& vs) { return Argument(Strings::trim(vs.token())); };
			m_parser["ContextArgument"] = [](const SV& vs) { return Argument(std::any_cast<Context>(vs[0])); };

			m_parser["ArgumentList"] = [](const SV& vs) {
				Arguments result;

				for (const auto& value : vs)
				{
					result.emplace_back(std::any_cast<Argument>(value));
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
					if (const auto* absolute = std::any_cast<StringView>(&value); absolute && *absolute == ".")
					{
						result.emplace_back(Operation{*absolute, {}});
						continue;
					}

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

			// Assignment -> Statement
			m_parser["Assignment"] = [](const SV& vs) {
				Statement result;

				result.kind    = Statement::Kind::Assignment;
				result.context = std::any_cast<Context>(vs[0]);
				result.value   = std::any_cast<Argument>(vs[1]);

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

	struct ExecutionContext
	{
		void* object             = nullptr;
		Refl::Object* reflection = nullptr;
		ExecutionFlags flags     = ExecutionFlags::Undefined;
		bool is_const            = false;
		bool is_read_only        = false;

		inline bool is_valid() const
		{
			if (reflection)
			{
				if (reflection->is<Refl::Scope>())
				{
					return true;
				}

				return object != nullptr;
			}

			return false;
		}
	};

	class BoundProperty final : public Refl::Property
	{
	private:
		void* m_address;

	public:
		BoundProperty(Name name, void* address, Refl::Type* type, EntryFlags flags, Refl::Object* owner)
		    : Property(name, type,
		               (static_cast<u64>(flags) << console_flags_shift)
		                       | (flags.any(EntryFlags::ReadOnly) ? Refl::Property::IsReadOnly : 0),
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

	class BoundFunction final : public Refl::Object
	{
	private:
		Ref<Refl::Function> m_function;

	public:
		BoundFunction(Name name, Refl::Function* function, EntryFlags flags, Refl::Object* owner)
		    : Object(name, static_cast<u64>(flags) << console_flags_shift, owner), m_function(retain_ref(function))
		{}

		Object* resolve(const Refl::Class* base) override
		{
			if (Object* object = Object::resolve(base))
				return object;

			return m_function->resolve(base);
		}
	};

	static Refl::Scope* root()
	{
		static Refl::Scope scope("Console");
		return &scope;
	}

	static Refl::Scope* scope_of(StringView path, bool create)
	{
		Refl::Scope* scope = root();

		for (usize begin = 0; scope != nullptr && begin < path.size();)
		{
			const usize end = path.find('.', begin);
			const Name name(path.substr(begin, end - begin));
			if (!name.is_valid())
				return nullptr;

			if (Refl::Object* object = scope->find(name))
			{
				scope = object->resolve<Refl::Scope>();
			}
			else if (create)
			{
				scope = Ref<Refl::Scope>::make(name, Refl::Object::None, scope).value();
			}
			else
			{
				return nullptr;
			}

			begin = end == StringView::npos ? path.size() : end + 1;
		}

		return scope;
	}

	static Refl::Object* find_binding(StringView name)
	{
		const usize position = name.rfind('.');
		Refl::Scope* scope   = scope_of(position == StringView::npos ? "" : name.substr(0, position), false);
		return scope ? scope->find(Name(name.substr(position == StringView::npos ? 0 : position + 1))) : nullptr;
	}

	static DynamicValue value_of(Refl::Type* type, const void* value)
	{
		DynamicValue result(type);
		return result && value != nullptr && type->copy_assign(result.data(), value) ? result : DynamicValue();
	}

	static DynamicValue execute_statement(ExecutionContext ctx, const Statement& statement);

	static DynamicValue resolve_value(ExecutionContext ctx, const Argument& source, Refl::Type* type)
	{
		if (const StringView* value = etl::get_if<StringView>(&source))
		{
			DynamicValue result(type);
			return result && type->parse(result.data(), *value) == value->size() ? result : DynamicValue();
		}

		const Context* value = etl::get_if<Context>(&source);
		if (value == nullptr)
			return {};

		Statement statement;
		statement.context = *value;

		DynamicValue result = execute_statement(ctx, statement);
		return result && result.type() == type ? result : DynamicValue();
	}

	static DynamicValue execute_function(const ExecutionContext& ctx, Refl::Function* function, Span<const Argument> arguments)
	{
		const auto& parameters = function->args();
		bool has_this          = false;

		for (const Ref<Refl::Property>& parameter : parameters)
		{
			has_this = has_this || parameter->is_this();
		}

		{
			if (ctx.is_const && function->is_method() && !function->is_const())
			{
				trinex_warning(Log::Console, "Cannot call a non-const method on a const object");
				return {};
			}

			usize required_arguments = function->args().size();

			if (ctx.object != nullptr)
			{
				for (const Ref<Refl::Property>& prop : function->args())
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
		Array<Refl::Property*> constructed;
		bool success = true;

		for (usize index = 0, argument_index = 0; success && index < parameters.size(); ++index)
		{
			Refl::Property* parameter = parameters[index].value();
			void* value               = parameter->address(storage);
			success                   = value != nullptr && parameter->type()->construct(value);

			if (!success)
				break;

			constructed.emplace_back(parameter);

			if (parameter->is_this())
			{
				success = parameter->type()->copy_assign(value, &ctx.object);
			}
			else
			{
				const Argument& argument = arguments[argument_index++];
				DynamicValue resolved    = resolve_value(ctx, argument, const_cast<Refl::Type*>(parameter->type()));
				success                  = resolved && resolved.type() == parameter->type()
				                           && parameter->type()->copy_assign(value, resolved.data());
			}
		}

		DynamicValue result;

		if (success)
		{
			function->invoke(storage);
			if (Refl::Property* return_value = function->result())
				result = value_of(const_cast<Refl::Type*>(return_value->type()), return_value->load(storage));
		}
		else
		{
			trinex_warning(Log::Console, "Failed to prepare function arguments");
		}

		while (!constructed.empty())
		{
			Refl::Property* parameter = constructed.back();
			parameter->type()->destruct(parameter->address(storage));
			constructed.pop_back();
		}

		ByteAllocator::deallocate(static_cast<unsigned char*>(storage));
		return result;
	}

	template<typename F>
	static DynamicValue execute_operations(ExecutionContext ctx, Span<const Operation> operations, F&& func)
	{
		if (!ctx.is_valid())
			return {};

		const ExecutionContext base_context = ctx;

		Array<DynamicValue> result;
		result.reserve(operations.size());

		while (!operations.empty())
		{
			const Operation* operation = operations.data();
			operations                 = operations.subspan(1);

			if (operation->name == ".")
			{
				ctx.object       = nullptr;
				ctx.reflection   = root();
				ctx.is_const     = false;
				ctx.is_read_only = false;
				continue;
			}

			if (Refl::IndirectionType* indirect = ctx.reflection->cast<Refl::IndirectionType>())
			{
				ctx.object     = ctx.object ? *static_cast<void**>(ctx.object) : nullptr;
				ctx.reflection = indirect->type();
				ctx.is_const   = ctx.is_const || ctx.reflection->is<Refl::ConstType>();
			}

			if (ctx.reflection == nullptr || (ctx.object == nullptr && !ctx.reflection->is<Refl::Scope>()))
				return {};

			Refl::Object* entry = ctx.reflection->find(operation->name);

			if (entry == nullptr)
				return {};

			if (Refl::Function* function = entry->resolve<Refl::Function>())
			{
				DynamicValue& value = result.emplace_back(execute_function(base_context, function, operation->arguments));

				if (function->result() == nullptr)
				{
					if (!operations.empty())
						return {};
					return func(ctx);
				}

				if (!value)
					return {};

				ctx.object       = value.data();
				ctx.reflection   = value.type();
				ctx.is_const     = ctx.reflection->is<Refl::ConstType>();
				ctx.is_read_only = false;
			}
			else if (Refl::Property* property = entry->resolve<Refl::Property>())
			{
				ctx.object       = property->address(ctx.object);
				ctx.reflection   = const_cast<Refl::Type*>(property->type());
				ctx.is_const     = ctx.is_const || ctx.reflection->is<Refl::ConstType>();
				ctx.is_read_only = property->is_read_only();

				if (ctx.object == nullptr)
					return {};
			}
			else if (Refl::Scope* child = entry->resolve<Refl::Scope>())
			{
				ctx.object       = nullptr;
				ctx.reflection   = child;
				ctx.is_const     = false;
				ctx.is_read_only = false;
			}
			else
			{
				return {};
			}
		}

		return func(ctx);
	}

	static DynamicValue execute_statement(ExecutionContext ctx, const Statement& statement)
	{
		return execute_operations(ctx, statement.context, [&statement, &base = ctx](const ExecutionContext& ctx) {
			if (!ctx.is_valid())
			{
				if (!ctx.flags.any(ExecutionFlags::IgnoreUnknown))
					trinex_warning(Log::Console, "Failed to resolve console context");
				return DynamicValue();
			}

			if (statement.kind == Statement::Kind::Scope)
			{
				if (!ctx.reflection->is<Refl::Scope>())
				{
					trinex_warning(Log::Console, "Console context is not a scope");
					return DynamicValue();
				}

				DynamicValue result;

				for (const Statement& child : statement.children)
				{
					result = execute_statement(ctx, child);
				}

				return result;
			}

			Refl::Type* type = ctx.reflection->resolve<Refl::Type>();

			if (type == nullptr || ctx.object == nullptr)
				return DynamicValue();

			if (statement.kind == Statement::Kind::Assignment)
			{
				if (ctx.is_const || ctx.is_read_only)
				{
					trinex_warning(Log::Console, "Cannot assign to a const or read-only console value");
					return DynamicValue();
				}

				DynamicValue value = resolve_value(base, statement.value, type);

				if (!value || value.type() != type || !type->copy_assign(ctx.object, value.data()))
				{
					trinex_warning(Log::Console, "Failed to assign console value");
					return DynamicValue();
				}
			}

			return value_of(type, ctx.object);
		});
	}

	trinex_api DynamicFunction resolve_function(const Refl::Function* function)
	{
		if (function == nullptr)
			return nullptr;

		if (const Refl::NativeFunction* native = function->cast<Refl::NativeFunction>())
		{
			return native->address();
		}

		return nullptr;
	}

	trinex_api void property(StringView name, void* address, Refl::Type* reflection, StringView description, EntryFlags flags)
	{
		const usize position = name.rfind('.');
		Refl::Scope* scope   = scope_of(position == StringView::npos ? "" : name.substr(0, position), true);
		const Name binding_name(name.substr(position == StringView::npos ? 0 : position + 1));
		trinex_verify(address != nullptr && reflection != nullptr && scope != nullptr && binding_name.is_valid()
		              && find_binding(name) == nullptr);
		(void) Ref<BoundProperty>::make(binding_name, address, reflection, flags, scope);
		(void) description;
	}

	trinex_api void function(StringView name, Refl::Function* function, StringView description, EntryFlags flags)
	{
		const usize position = name.rfind('.');
		Refl::Scope* scope   = scope_of(position == StringView::npos ? "" : name.substr(0, position), true);
		const Name binding_name(name.substr(position == StringView::npos ? 0 : position + 1));
		trinex_verify(function != nullptr && scope != nullptr && binding_name.is_valid() && find_binding(name) == nullptr);
		(void) Ref<BoundFunction>::make(binding_name, function, flags, scope);
		(void) description;
	}

	trinex_api void function(StringView name, DynamicFunction func, DynamicInvoker invoker, Refl::Type* result, Refl::Type** args,
	                         usize count, StringView description, EntryFlags flags)
	{
		auto native = Ref<Refl::NativeFunction>::make(name, func, invoker, result, args, count);
		function(name, native.value(), description, flags);
	}

	trinex_api void unbind(StringView name)
	{
		if (Refl::Object* binding = find_binding(name))
			binding->owner(nullptr);
	}

	trinex_api DynamicValue execute(StringView source, ExecutionFlags flags)
	{
		static thread_local Parser parser;

		Statements statements;

		if (parser.parse(source, statements))
		{
			DynamicValue result;

			ExecutionContext ctx = {nullptr, root(), flags};

			for (const Statement& statement : statements)
			{
				result = execute_statement(ctx, statement);
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
} // namespace Trinex::Console
