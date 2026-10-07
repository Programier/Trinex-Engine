#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Reflector
{
	enum class DiagnosticSeverity : std::uint8_t
	{
		Error,
		Warning,
	};

	enum class Access : std::uint8_t
	{
		Global,
		Private,
		Protected,
		Public,
	};

	enum PropertyFlags : std::uint16_t
	{
		PropertyFlag_None      = 0,
		PropertyFlag_Static    = 1 << 0,
		PropertyFlag_Const     = 1 << 1,
		PropertyFlag_Constexpr = 1 << 2,
		PropertyFlag_Mutable   = 1 << 3,
		PropertyFlag_Pointer   = 1 << 4,
		PropertyFlag_Reference = 1 << 5,
	};

	enum TypeFlags : std::uint16_t
	{
		TypeFlag_None      = 0,
		TypeFlag_Const     = 1 << 0,
		TypeFlag_Volatile  = 1 << 1,
		TypeFlag_Pointer   = 1 << 2,
		TypeFlag_Reference = 1 << 3,
		TypeFlag_RValueRef = 1 << 4,
		TypeFlag_Template  = 1 << 5,
	};

	struct TypeInfo {
		std::string raw;
		std::string name;
		std::string qualified_name;
		std::vector<std::string> namespaces;
		std::vector<TypeInfo> template_arguments;
		std::uint16_t flags        = TypeFlag_None;
		std::uint8_t pointer_depth = 0;
	};

	enum FunctionFlags : std::uint16_t
	{
		FunctionFlag_None        = 0,
		FunctionFlag_Static      = 1 << 0,
		FunctionFlag_Virtual     = 1 << 1,
		FunctionFlag_Const       = 1 << 2,
		FunctionFlag_Constexpr   = 1 << 3,
		FunctionFlag_Inline      = 1 << 4,
		FunctionFlag_Noexcept    = 1 << 5,
		FunctionFlag_Override    = 1 << 6,
		FunctionFlag_Final       = 1 << 7,
		FunctionFlag_PureVirtual = 1 << 8,
		FunctionFlag_Constructor = 1 << 9,
		FunctionFlag_Destructor  = 1 << 10,
		FunctionFlag_Operator    = 1 << 11,
		FunctionFlag_Template    = 1 << 12,
	};

	struct Diagnostic {
		DiagnosticSeverity severity = DiagnosticSeverity::Error;
		std::string message;
		std::size_t line   = 0;
		std::size_t column = 0;
	};

	struct Annotation {
		struct Argument {
			std::string name;
			std::string value;
		};

		std::string name;
		std::string arguments;
		std::size_t line   = 0;
		std::size_t column = 0;
		std::vector<Argument> metadata;
	};

	struct Property {
		Annotation annotation;
		std::string name;
		std::string type;
		TypeInfo type_info;
		std::string owner;
		std::string full_name;
		Access access = Access::Global;
		std::string default_value;
		std::string attributes;
		std::string declaration;
		std::string engine_macros;
		std::uint64_t line;
		std::uint16_t flags = PropertyFlag_None;
	};

	struct Function {
		struct Parameter {
			std::string name;
			std::string type;
			TypeInfo type_info;
			std::string default_value;
			std::string declaration;
		};

		Annotation annotation;
		std::string name;
		std::string owner;
		std::string full_name;
		std::string return_type;
		TypeInfo return_type_info;
		std::string parameters;
		std::string qualifiers;
		std::string attributes;
		std::string declaration;
		std::string template_prefix;
		std::string engine_macros;
		std::vector<Parameter> parsed_parameters;

		Access access       = Access::Global;
		std::uint64_t line  = 0;
		std::uint16_t flags = FunctionFlag_None;
	};

	struct Type {
		Annotation annotation;
		std::string kind;
		std::string name;
		std::string scope;
		std::string full_name;
		std::string bases;
		std::string attributes;
		std::string declaration;
		std::string engine_macros;
		std::string template_prefix;
		std::vector<Type> nested_types;
		std::vector<Property> properties;
		std::vector<Function> functions;
		std::vector<std::string> enum_values;
		TypeFlags flags = TypeFlag_None;
		std::size_t line;
	};

	struct TranslationUnit {
		std::string source_name;
		std::vector<Type> types;
		std::vector<Property> properties;
		std::vector<Function> functions;
		std::vector<Diagnostic> diagnostics;
	};
}// namespace Reflector
