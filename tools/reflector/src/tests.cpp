#include <model.hpp>
#include <parser.hpp>
#include <printer.hpp>
#include <stream.hpp>

#include <iostream>
#include <memory>
#include <sstream>
#include <string_view>

namespace
{
	using namespace Reflector;

	bool expect(bool value, std::string_view message)
	{
		if (!value)
			std::cerr << "FAILED: " << message << '\n';
		return value;
	}

	template<typename T>
	T* find(Scope& scope, std::string_view name, ObjectKind kind)
	{
		return dynamic_cast<T*>(scope.find(name, kind));
	}

	bool test_code_writer()
	{
		CodeWriter writer("  ");
		writer.line("namespace Test");
		writer.line("{");
		{
			auto indentation = writer.scoped_indent();
			writer.write("int value;\r\n\nfloat other;");
			writer.line();
		}
		writer.line("}");
		if (!expect(writer.text() == "namespace Test\n{\n  int value;\n\n  float other;\n}\n", "code writer formatting"))
			return false;

		writer.indent().indent().unindent();
		return expect(writer.indent_level() == 1, "code writer indentation level") &&
		       expect(writer.write("tail").text().ends_with("  tail"), "code writer writes after indentation changes");
	}

	bool test_preprocessor_continuations()
	{
		for (const std::string newline : {"\n", "\r\n"})
		{
			const std::string directive = "  #define trinex_material_parameter(self, super) \\" + newline +
			                              "\ttrinex_class(self, super); \\" + newline + "\t\\" + newline;
			const std::string source = directive + newline + "trinex_property() int visible;";
			const auto tokens = tokenize(source);
			if (!expect(tokens[0].type == TokenType::Preprocessor &&
			                    tokens[0].value.find("trinex_class(self, super);") != std::string_view::npos,
			            "continued directive is a single token") ||
			    !expect(tokens[1].value == "trinex_property" && tokens[1].line == 5 && tokens[1].column == 1,
			            "declaration position after continued directive"))
				return false;
			std::unique_ptr<Module> module(parse(source));
			if (!expect(module && module->objects.size() == 1 && find<Property>(*module, "visible", ObjectKind::Property),
			            "macro body is ignored without skipping the following declaration"))
				return false;
		}
		for (const std::string_view source : {"#define MACRO trinex_class(self, super);",
		                                     "#define MACRO \\\ntrinex_class(self, super);",
		                                     "#define MACRO \\\ntrinex_class(self, super); \\"})
		{
			std::unique_ptr<Module> module(parse(source));
			if (!expect(module && module->objects.empty(), "directive at end of file is ignored"))
				return false;
		}
		return true;
	}

	bool test_namespace_context()
	{
		std::unique_ptr<Module> module(parse(R"(
namespace Engine::Gameplay {
	trinex_class(DisplayName="Actor") class Actor {
	public:
		trinex_property(Category="State") int value;
		trinex_function() void tick(float dt);
	};
	trinex_function() int sum(int a, int b);
}
namespace Engine { namespace Gameplay {
	trinex_property() int count = 4;
}}
namespace Alias = Engine::Gameplay;
namespace { trinex_property() int hidden; }
namespace { trinex_function() void local(); }
)",
		                                     "namespace_context"));
		if (!expect(module != nullptr, "source parses successfully"))
			return false;
		auto* actor = find<Class>(*module, "Engine::Gameplay::Actor", ObjectKind::Class);
		auto* space = find<Namespace>(*module, "Engine::Gameplay", ObjectKind::Namespace);
		auto* value = find<Property>(*module, "Engine::Gameplay::Actor::value", ObjectKind::Property);
		auto* tick  = find<Function>(*module, "Engine::Gameplay::Actor::tick", ObjectKind::Function);
		if (!expect(actor && space && value && tick, "qualified lookup through owning scopes"))
			return false;
		return expect(module->name == "namespace_context" && module->owner == nullptr, "module identity") &&
		       expect(module->is_a(ObjectKind::Module), "module object kind") &&
		       expect(actor->owner == space && value->owner == actor && tick->owner == actor, "parent pointers") &&
		       expect(actor->objects.size() == 2 && space->objects.size() == 3, "namespace reopening merges children") &&
		       expect(value->access == Access::Public && value->type.name == "int", "property type and access") &&
		       expect(tick->args.size() == 1 && tick->args[0].name == "dt" && tick->args[0].type.name == "float", "argument") &&
		       expect(actor->metadata[0].name == "DisplayName" && value->metadata[0].value == "\"State\"", "metadata") &&
		       expect(tick->find("missing", ObjectKind::Property) == nullptr, "leaf lookup") &&
		       expect(actor->find("::Engine::Gameplay::Actor", ObjectKind::Class) == actor, "absolute lookup") &&
		       expect(module->find("Engine::Gameplay::Actor", ObjectKind::Struct) == nullptr, "lookup checks object kind") &&
		       expect(module->objects.size() == 2 && static_cast<Scope&>(*module->objects[1]).objects.size() == 2,
		              "anonymous namespace ownership");
	}

	bool test_complex_declarations()
	{
		std::unique_ptr<Module> module(parse(R"(
trinex_class() class Source {
public:
	trinex_property() std::vector<std::pair<int, float>>& value = GetValue();
	trinex_property() int (*callback)(float);
	trinex_property() int* const pointer = nullptr;
	trinex_function() int method(int (*func)(int a, int b), const std::vector<int>& values) const;
	trinex_function() void unnamed(int, const Source&, std::vector<int>, unsigned long, int count = 8);
};
)"));
		if (!expect(module != nullptr, "source parses successfully"))
			return false;
		auto* value    = find<Property>(*module, "Source::value", ObjectKind::Property);
		auto* callback = find<Property>(*module, "Source::callback", ObjectKind::Property);
		auto* pointer  = find<Property>(*module, "Source::pointer", ObjectKind::Property);
		auto* method   = find<Function>(*module, "Source::method", ObjectKind::Function);
		auto* unnamed  = find<Function>(*module, "Source::unnamed", ObjectKind::Function);
		if (!expect(value && callback && pointer && method && unnamed, "complex declarations exist"))
			return false;
		const auto& type = value->type;
		if (!expect(type.name == "std::vector" && type.templates.size() == 1, "template type") ||
		    !expect(type.templates[0].type.name == "std::pair" && type.templates[0].type.templates.size() == 2,
		            "nested template arguments"))
			return false;
		return expect(type.flags == (TypeInfo::Template | TypeInfo::Reference), "type context flags") &&
		       expect(value->value == "GetValue()", "property initializer") &&
		       expect(callback->type.name == "int (*)(float)", "function pointer retained") &&
		       expect(pointer->type.name == "int" && pointer->type.flags == (TypeInfo::Pointer | TypeInfo::PointerConst),
		              "pointer cv retained") &&
		       expect(method->args.size() == 2 && method->args[0].type.name == "int (*)(int a, int b)", "callback argument") &&
		       expect(method->args[1].type.flags == (TypeInfo::Const | TypeInfo::Reference | TypeInfo::Template),
		              "argument qualifiers") &&
		       expect(method->flags == Function::Const, "argument const does not leak into function flags") &&
		       expect(unnamed->args.size() == 5 && unnamed->args[0].name.empty() && unnamed->args[1].name.empty() &&
		                      unnamed->args[2].name.empty() && unnamed->args[3].name.empty(),
		              "unnamed arguments") &&
		       expect(unnamed->args[4].name == "count" && unnamed->args[4].value == "8", "default argument");
	}

	bool test_pointer_types()
	{
		std::unique_ptr<Module> module(parse(R"(
trinex_property() const std::vector<int>* items;
trinex_property() volatile int* const volatile qualified;
trinex_property() const int numbers[8];
trinex_property() std::vector<int*> nested;
trinex_property() std::vector<int[4]> nested_array;
trinex_function() std::vector<int>* read(const int values[4], int* const& pointer, const std::vector<int>* = nullptr);
trinex_function() auto trailing() -> int*;
)"));
		if (!expect(module != nullptr, "source parses successfully"))
			return false;
		auto* items        = find<Property>(*module, "items", ObjectKind::Property);
		auto* qualified    = find<Property>(*module, "qualified", ObjectKind::Property);
		auto* numbers      = find<Property>(*module, "numbers", ObjectKind::Property);
		auto* nested       = find<Property>(*module, "nested", ObjectKind::Property);
		auto* nested_array = find<Property>(*module, "nested_array", ObjectKind::Property);
		auto* read         = find<Function>(*module, "read", ObjectKind::Function);
		auto* trailing     = find<Function>(*module, "trailing", ObjectKind::Function);
		if (!expect(items && qualified && numbers && nested && nested_array && read && trailing, "pointer declarations"))
			return false;
		if (!expect(items->type.name == "std::vector" && items->type.templates.size() == 1 &&
		                    items->type.flags == (TypeInfo::Const | TypeInfo::Template | TypeInfo::Pointer),
		            "pointer retains structured template arguments"))
			return false;
		if (!expect(qualified->type.name == "int" &&
		                    qualified->type.flags ==
		                            (TypeInfo::Volatile | TypeInfo::Pointer | TypeInfo::PointerConst | TypeInfo::PointerVolatile),
		            "separate pointee and pointer qualifiers"))
			return false;
		if (!expect(numbers->type.name == "int" && numbers->type.flags == (TypeInfo::Const | TypeInfo::Pointer), "array decay"))
			return false;
		if (!expect(nested->type.templates.size() == 1 && nested->type.templates[0].type.flags == TypeInfo::Pointer &&
		                    nested_array->type.templates.size() == 1 &&
		                    nested_array->type.templates[0].type.flags == TypeInfo::Pointer,
		            "nested type pointer flags"))
			return false;
		if (!expect(read->type.flags == (TypeInfo::Template | TypeInfo::Pointer) && read->args.size() == 3 &&
		                    read->args[0].type.flags == (TypeInfo::Const | TypeInfo::Pointer) &&
		                    read->args[1].type.flags == (TypeInfo::Pointer | TypeInfo::PointerConst | TypeInfo::Reference) &&
		                    read->args[2].name.empty() && read->args[2].type.templates.size() == 1 &&
		                    trailing->type.flags == TypeInfo::Pointer,
		            "function pointer types"))
			return false;
		std::ostringstream output;
		Reflector::print(output, module.get());
		if (!expect(output.str().find("const std::vector<int>*") != std::string::npos &&
		                    output.str().find("pointer_const") != std::string::npos,
		            "printer pointer flags"))
			return false;
		for (const auto declaration :
		     {"trinex_property() int** invalid;", "trinex_property() int* const* invalid;", "trinex_property() int* invalid[4];",
		      "trinex_property() int invalid[4][8];", "trinex_property() int (*invalid)[4];",
		      "trinex_property() std::vector<int**> invalid;", "trinex_property() Outer<int**>::Inner invalid;",
		      "trinex_property() std::vector<int*[4]> invalid;", "trinex_function() int** invalid();",
		      "trinex_function() auto invalid() -> int**;", "trinex_function() void invalid(int** arg);",
		      "trinex_function() void invalid(int* args[4]);", "trinex_function() void invalid(int (*callback)(int**));",
		      "trinex_property() int (**invalid)(float);"})
		{
			std::unique_ptr<Module> rejected(parse(std::string(declaration) + "\ntrinex_property() int* valid;"));
			if (!expect(rejected == nullptr, declaration))
				return false;
		}
		return true;
	}

	bool test_special_functions()
	{
		std::unique_ptr<Module> module(parse(R"(
namespace Engine {
	trinex_class() class Box {
	public:
		trinex_function() Box();
		trinex_function() ~Box();
		trinex_function() bool operator==(const Box& other) const;
		trinex_function() operator bool() const;
		trinex_function() int operator()() const;
		trinex_function() int& operator[](int index);
	};
}
)"));
		if (!expect(module != nullptr, "source parses successfully"))
			return false;
		auto* box       = find<Class>(*module, "Engine::Box", ObjectKind::Class);
		auto* ctor      = find<Function>(*module, "Engine::Box::Box", ObjectKind::Function);
		auto* dtor      = find<Function>(*module, "Engine::Box::~Box", ObjectKind::Function);
		auto* eq        = find<Function>(*module, "Engine::Box::operator==", ObjectKind::Function);
		auto* convert   = find<Function>(*module, "Engine::Box::operator bool", ObjectKind::Function);
		auto* call      = find<Function>(*module, "Engine::Box::operator()", ObjectKind::Function);
		auto* subscript = find<Function>(*module, "Engine::Box::operator[]", ObjectKind::Function);
		if (!expect(box && ctor && dtor && eq && convert && call && subscript, "special functions exist"))
			return false;
		return expect(box->objects.size() == 6, "special function count") &&
		       expect(ctor->flags & Function::Constructor, "constructor flag") &&
		       expect(dtor->flags & Function::Destructor, "destructor flag") &&
		       expect(eq->type.name == "bool" && (eq->flags & Function::Operator), "operator") &&
		       expect(convert->type.name == "bool", "conversion target") &&
		       expect(call->args.empty() && (call->flags & Function::Const), "call operator") &&
		       expect(subscript->type.flags & TypeInfo::Reference, "subscript return reference");
	}

	bool test_attributes_macros_and_nested_types()
	{
		std::unique_ptr<Module> module(parse(R"(
namespace Engine {
	trinex_class() [[nodiscard]] class TRINEX_API Outer final : public virtual Base<int>, Other {
		GENERATED_BODY()
	public:
		trinex_property() [[maybe_unused]] alignas(16) int value;
		trinex_function() [[nodiscard]] FORCE_INLINE int get() const;
		trinex_struct() struct Inner {
			trinex_property() int nested_value;
		};
	protected:
		trinex_property() mutable int state;
	private:
		trinex_function() virtual void work() volatile && noexcept override final = 0;
	};
}
)"));
		if (!expect(module != nullptr, "source parses successfully"))
			return false;
		auto* outer = find<Class>(*module, "Engine::Outer", ObjectKind::Class);
		auto* inner = find<Struct>(*module, "Engine::Outer::Inner", ObjectKind::Struct);
		auto* value = find<Property>(*module, "Engine::Outer::Inner::nested_value", ObjectKind::Property);
		auto* state = find<Property>(*module, "Engine::Outer::state", ObjectKind::Property);
		auto* get   = find<Function>(*module, "Engine::Outer::get", ObjectKind::Function);
		auto* work  = find<Function>(*module, "Engine::Outer::work", ObjectKind::Function);
		if (!expect(outer && inner && value && state && get && work, "attributes and nested types"))
			return false;
		return expect(outer->flags == Struct::Final && outer->bases.size() == 2, "final and bases") &&
		       expect(outer->bases[0].type.name == "Base" && outer->bases[0].type.templates.size() == 1 &&
		                      outer->bases[0].flags == Struct::Base::Virtual && outer->bases[0].access == Access::Public,
		              "virtual template base") &&
		       expect(outer->bases[1].access == Access::Private, "class default base access") &&
		       expect(value->owner == inner && value->access == Access::Public, "nested ownership and struct access") &&
		       expect(state->access == Access::Protected && state->flags == Property::Mutable, "member flags and access") &&
		       expect(get->flags == (Function::Inline | Function::Const), "inline macro") &&
		       expect(work->flags == (Function::Virtual | Function::Volatile | Function::RValueRef | Function::Noexcept |
		                              Function::Override | Function::Final | Function::PureVirtual),
		              "function qualifiers");
	}

	bool test_template_arguments_and_flags()
	{
		std::unique_ptr<Module> module(parse(R"(
trinex_struct() struct Data {
	trinex_property() std::array<std::vector<int>, 4> items;
	trinex_property() Tuple<Ts...> pack;
	trinex_property() Empty<> empty;
	trinex_property() inline static constexpr int limit = 16;
	trinex_property() unsigned int bits : 3 = 2;
	trinex_property() int values[2] {1, 2};
	trinex_function() static void log(const char* format, ...) noexcept(false);
	trinex_function() const Data& get() & noexcept(true);
	trinex_function() auto count() const -> const int&;
};
)"));
		if (!expect(module != nullptr, "source parses successfully"))
			return false;
		auto* items  = find<Property>(*module, "Data::items", ObjectKind::Property);
		auto* pack   = find<Property>(*module, "Data::pack", ObjectKind::Property);
		auto* empty  = find<Property>(*module, "Data::empty", ObjectKind::Property);
		auto* limit  = find<Property>(*module, "Data::limit", ObjectKind::Property);
		auto* bits   = find<Property>(*module, "Data::bits", ObjectKind::Property);
		auto* values = find<Property>(*module, "Data::values", ObjectKind::Property);
		auto* log    = find<Function>(*module, "Data::log", ObjectKind::Function);
		auto* get    = find<Function>(*module, "Data::get", ObjectKind::Function);
		auto* count  = find<Function>(*module, "Data::count", ObjectKind::Function);
		if (!expect(items && pack && empty && limit && bits && values && log && get && count, "template/flag declarations"))
			return false;
		if (!expect(items->type.templates.size() == 2 && pack->type.templates.size() == 1, "argument counts"))
			return false;
		return expect(items->type.templates[0].type.name == "std::vector", "template type argument") &&
		       expect(items->type.templates[1].flags == TypeInfo::TemplateArgument::Value &&
		                      items->type.templates[1].value == "4",
		              "non-type template argument") &&
		       expect(pack->type.templates[0].flags & TypeInfo::TemplateArgument::PackExpansion, "pack expansion") &&
		       expect(empty->type.templates.empty() && (empty->type.flags & TypeInfo::Template), "empty template list") &&
		       expect(limit->flags == (Property::Inline | Property::Static | Property::Constexpr), "property context flags") &&
		       expect(bits->flags == Property::Bitfield && bits->value == "2", "bit field") &&
		       expect(values->value == "{1, 2}" && values->type.name == "int" && values->type.flags == TypeInfo::Pointer,
		              "array initializer") &&
		       expect(log->flags == (Function::Static | Function::Variadic), "noexcept false and variadic") &&
		       expect(get->flags == (Function::Reference | Function::Noexcept) &&
		                      get->type.flags == (TypeInfo::Const | TypeInfo::Reference),
		              "return vs function qualifiers") &&
		       expect(count->type.name == "int" && count->type.flags == (TypeInfo::Const | TypeInfo::Reference) &&
		                      count->flags == Function::Const,
		              "trailing return type");
	}

	bool test_enums()
	{
		std::unique_ptr<Module> module(parse(R"(
namespace Test {
	trinex_enum() enum class Flags : unsigned {
		Zero, One = 1 << 2, Two, Both = One | Two, Negative = -2,
		Next, Hex = 0x10u, Bin = 0b11, Math = (Hex + 4) / 2,
		trinex_value(DisplayName="Last") Last = Flags::Math + 1
	};
}
)"));
		if (!expect(module != nullptr, "source parses successfully"))
			return false;
		auto* enumeration = find<Enum>(*module, "Test::Flags", ObjectKind::Enum);
		if (!expect(enumeration && enumeration->values.size() == 10, "enum values"))
			return false;
		const std::int64_t expected[] = {0, 4, 5, 5, -2, -1, 16, 3, 10, 11};
		for (std::size_t i = 0; i < enumeration->values.size(); ++i)
			if (!expect(enumeration->values[i].value == expected[i], enumeration->values[i].name))
				return false;
		return expect(enumeration->values.back().metadata.size() == 1, "enumerator metadata");
	}

	bool test_errors_and_reuse()
	{
		std::unique_ptr<Module> valid(parse("trinex_struct() struct Good { trinex_property() int value; };", "good.hpp"));
		if (!expect(valid != nullptr, "valid source parses successfully"))
			return false;
		auto* good = find<Struct>(*valid, "Good", ObjectKind::Struct);
		if (!expect(good && good->owner == valid.get(), "owning tree"))
			return false;
		std::ostringstream output;
		Reflector::print(output, valid.get());
		if (!expect(output.str().find("struct Good") != std::string::npos &&
		                    output.str().find("property value: int") != std::string::npos,
		            "printer traverses module"))
			return false;
		for (std::string_view source :
		     {"trinex_function() int broken;", "trinex_property() int;",
		      "trinex_enum() enum Bad { Unknown = External(), Following, Recovered = 8, Next };",
		      "template <typename T> trinex_class() class Box {};", "template <typename T> trinex_function() T cast();",
		      "trinex_class() class Broken {", "namespace Broken {", "trinex_property(", "trinex_function() void f(",
		      "trinex_property() int value"})
		{
			std::unique_ptr<Module> invalid(parse(source, "bad.hpp"));
			if (!expect(invalid == nullptr, source))
				return false;
			std::unique_ptr<Module> recovered(parse("trinex_property() int value;", "recovered.hpp"));
			if (!expect(recovered && find<Property>(*recovered, "value", ObjectKind::Property), "parser reuse after error"))
				return false;
		}
		if (!expect(valid->name == "good.hpp" && find<Struct>(*valid, "Good", ObjectKind::Struct) == good &&
		                    good->owner == valid.get(),
		            "parser reuse preserves previous module"))
			return false;
		std::unique_ptr<Module> empty(parse(""));
		return expect(empty && empty->objects.empty(), "empty source");
	}

}// namespace

int main()
{
	std::cerr.setstate(std::ios::failbit);

	bool success = true;
	success      = test_code_writer() && success;
	success      = test_preprocessor_continuations() && success;
	success      = test_namespace_context() && success;
	success      = test_complex_declarations() && success;
	success      = test_pointer_types() && success;
	success      = test_special_functions() && success;
	success      = test_attributes_macros_and_nested_types() && success;
	success      = test_template_arguments_and_flags() && success;
	success      = test_enums() && success;
	success      = test_errors_and_reuse() && success;
	if (!success)
		return 1;
	std::cout << "All reflector parser tests passed\n";
	return 0;
}
