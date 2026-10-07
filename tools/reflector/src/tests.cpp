#include <model.hpp>
#include <parser.hpp>

#include <iostream>
#include <string_view>

namespace
{
	bool expect(bool value, std::string_view message)
	{
		if (!value)
		{
			std::cerr << "FAILED: " << message << '\n';
		}
		return value;
	}

	const Reflector::Type* find_type(const Reflector::TranslationUnit& unit, std::string_view full_name)
	{
		for (const auto& type : unit.types)
		{
			if (type.full_name == full_name)
			{
				return &type;
			}
		}
		return nullptr;
	}

	const Reflector::Function* find_function(const Reflector::TranslationUnit& unit, std::string_view full_name)
	{
		for (const auto& function : unit.functions)
		{
			if (function.full_name == full_name)
			{
				return &function;
			}
		}
		return nullptr;
	}

	bool has_diagnostic(const Reflector::TranslationUnit& unit, std::string_view text)
	{
		for (const auto& diagnostic : unit.diagnostics)
		{
			if (diagnostic.message.find(text) != std::string::npos)
			{
				return true;
			}
		}
		return false;
	}

	bool test_namespace_context()
	{
		static constexpr std::string_view source = R"(
namespace Engine::Gameplay {
	trinex_class()
	class Actor {
	public:
		trinex_property()
		int value;

		trinex_function()
		void tick(float dt);
	};

	trinex_function()
	int sum(int a, int b);
}
)";

		Reflector::Parser parser;
		const auto unit   = parser.parse(source, "namespace_context");
		const auto* actor = find_type(unit, "Engine::Gameplay::Actor");

		return expect(actor != nullptr, "type full name with namespace") &&
		       expect(actor->scope == "Engine::Gameplay", "type scope") &&
		       expect(actor->properties.size() == 1, "member property count") &&
		       expect(actor->properties[0].owner == "Engine::Gameplay::Actor", "member property owner") &&
		       expect(actor->properties[0].full_name == "Engine::Gameplay::Actor::value", "member property full name") &&
		       expect(actor->functions.size() == 1, "member function count") &&
		       expect(actor->functions[0].full_name == "Engine::Gameplay::Actor::tick", "member function full name") &&
		       expect(find_function(unit, "Engine::Gameplay::sum") != nullptr, "global function namespace full name");
	}

	bool test_complex_declarations()
	{
		static constexpr std::string_view source = R"(
trinex_class()
class Source {
public:
	trinex_property()
	std::vector<std::pair<int, float>>& value = GetValue();

	trinex_property()
	int (*callback)(float);

	trinex_function()
	int method(int (*func)(int a, int b), const std::vector<int>& values) const;
};
)";

		Reflector::Parser parser;
		const auto unit         = parser.parse(source, "complex_declarations");
		const auto* source_type = find_type(unit, "Source");

		return expect(source_type != nullptr, "complex type exists") &&
		       expect(source_type->properties.size() == 2, "complex property count") &&
		       expect(source_type->properties[0].name == "value", "template property name") &&
		       expect(source_type->properties[0].type == "std::vector<std::pair<int, float>>&", "template property type") &&
		       expect(source_type->properties[0].type_info.qualified_name == "std::vector", "template property qualified name") &&
		       expect(source_type->properties[0].type_info.name == "vector", "template property base name") &&
		       expect(source_type->properties[0].type_info.namespaces.size() == 1 &&
		                      source_type->properties[0].type_info.namespaces[0] == "std",
		              "template property namespace") &&
		       expect(source_type->properties[0].type_info.template_arguments.size() == 1, "template property argument count") &&
		       expect(source_type->properties[0].type_info.template_arguments[0].qualified_name == "std::pair",
		              "nested template qualified name") &&
		       expect(source_type->properties[0].type_info.flags & Reflector::TypeFlag_Reference,
		              "template property reference flag") &&
		       expect(source_type->properties[1].name == "callback", "function pointer property name") &&
		       expect(source_type->properties[1].type == "int (*)(float)", "function pointer property type") &&
		       expect(source_type->functions.size() == 1, "complex function count") &&
		       expect(source_type->functions[0].parsed_parameters.size() == 2, "complex function parameter count") &&
		       expect(source_type->functions[0].parsed_parameters[0].name == "func", "function pointer parameter name") &&
		       expect(source_type->functions[0].parsed_parameters[0].type == "int (*)(int a, int b)",
		              "function pointer parameter type") &&
		       expect(source_type->functions[0].parsed_parameters[1].type == "const std::vector<int>&",
		              "template reference parameter type") &&
		       expect(source_type->functions[0].parsed_parameters[1].type_info.qualified_name == "std::vector",
		              "parameter type qualified name") &&
		       expect(source_type->functions[0].parsed_parameters[1].type_info.flags & Reflector::TypeFlag_Const,
		              "parameter type const flag") &&
		       expect(source_type->functions[0].parsed_parameters[1].type_info.flags & Reflector::TypeFlag_Reference,
		              "parameter type reference flag");
	}

	bool test_templates_and_special_functions()
	{
		static constexpr std::string_view source = R"(
namespace Engine {
	template <typename T>
	trinex_class()
	class Box {
	public:
		trinex_function()
		Box();

		trinex_function()
		~Box();

		trinex_function()
		bool operator==(const Box& other) const;

		trinex_function()
		operator bool() const;

		template <typename U>
		trinex_function()
		U cast() const;
	};
}
)";

		Reflector::Parser parser;
		const auto unit = parser.parse(source, "templates_and_special_functions");
		const auto* box = find_type(unit, "Engine::Box");

		return expect(box != nullptr, "templated type exists") &&
		       expect(box->template_prefix == "template <typename T>", "type template prefix") &&
		       expect(box->functions.size() == 5, "special function count") &&
		       expect(box->functions[0].name == "Box", "constructor name") &&
		       expect(box->functions[0].flags & Reflector::FunctionFlag_Constructor, "constructor flag") &&
		       expect(box->functions[1].name == "~Box", "destructor name") &&
		       expect(box->functions[1].flags & Reflector::FunctionFlag_Destructor, "destructor flag") &&
		       expect(box->functions[2].name == "operator==", "operator== name") &&
		       expect(box->functions[2].return_type == "bool", "operator== return type") &&
		       expect(box->functions[2].flags & Reflector::FunctionFlag_Operator, "operator== flag") &&
		       expect(box->functions[3].name == "operator bool", "conversion operator name") &&
		       expect(box->functions[3].return_type.empty(), "conversion operator return type") &&
		       expect(box->functions[3].flags & Reflector::FunctionFlag_Operator, "conversion operator flag") &&
		       expect(box->functions[4].template_prefix == "template <typename U>", "function template prefix") &&
		       expect(box->functions[4].return_type == "U", "function template return type") &&
		       expect(has_diagnostic(unit, "reflected template classes are not supported"), "template class diagnostic") &&
		       expect(has_diagnostic(unit, "reflected template functions are not supported"), "template function diagnostic");
	}

	bool test_attributes_macros_and_nested_types()
	{
		static constexpr std::string_view source = R"(
namespace Engine {
	trinex_class()
	[[nodiscard]] class TRINEX_API Outer final {
		GENERATED_BODY()
	public:
		trinex_property()
		[[maybe_unused]] alignas(16) int value;

		trinex_function()
		[[nodiscard]] FORCE_INLINE int get() const;

		trinex_class()
		struct Inner {
			trinex_property()
			int nested_value;
		};
	};
}
)";

		Reflector::Parser parser;
		const auto unit   = parser.parse(source, "attributes_macros_nested");
		const auto* outer = find_type(unit, "Engine::Outer");

		return expect(outer != nullptr, "outer type exists") && expect(outer->attributes == "[[nodiscard]]", "type attribute") &&
		       expect(outer->engine_macros == "TRINEX_API", "type engine macro") &&
		       expect(outer->properties.size() == 1, "attribute property count") &&
		       expect(outer->properties[0].attributes == "[[maybe_unused]] alignas(16)", "property attributes") &&
		       expect(outer->properties[0].name == "value", "attribute property name") &&
		       expect(outer->functions.size() == 1, "attribute function count") &&
		       expect(outer->functions[0].attributes == "[[nodiscard]]", "function attribute") &&
		       expect(outer->functions[0].engine_macros == "FORCE_INLINE", "function macro") &&
		       expect(outer->nested_types.size() == 1, "nested type count") &&
		       expect(outer->nested_types[0].full_name == "Engine::Outer::Inner", "nested type full name") &&
		       expect(outer->nested_types[0].properties.size() == 1, "nested type property count") &&
		       expect(outer->nested_types[0].properties[0].full_name == "Engine::Outer::Inner::nested_value",
		              "nested property full name");
	}

	bool test_template_property_is_allowed()
	{
		static constexpr std::string_view source = R"(
trinex_class()
class Container {
	trinex_property()
	std::vector<int> values;
};
)";

		Reflector::Parser parser;
		const auto unit       = parser.parse(source, "template_property_allowed");
		const auto* container = find_type(unit, "Container");

		return expect(container != nullptr, "container type exists") &&
		       expect(container->properties.size() == 1, "template property count") &&
		       expect(container->properties[0].type == "std::vector<int>", "template property type") &&
		       expect(container->properties[0].type_info.qualified_name == "std::vector", "template property TypeInfo name") &&
		       expect(container->properties[0].type_info.template_arguments.size() == 1, "template property TypeInfo args") &&
		       expect(container->properties[0].type_info.template_arguments[0].qualified_name == "int",
		              "template property TypeInfo arg name") &&
		       expect(unit.diagnostics.empty(), "template property has no diagnostics");
	}

}// namespace

int main()
{
	bool success = true;
	success      = test_namespace_context() && success;
	success      = test_complex_declarations() && success;
	success      = test_templates_and_special_functions() && success;
	success      = test_attributes_macros_and_nested_types() && success;
	success      = test_template_property_is_allowed() && success;

	if (!success)
	{
		return 1;
	}

	std::cout << "All reflector parser tests passed\n";
	return 0;
}
