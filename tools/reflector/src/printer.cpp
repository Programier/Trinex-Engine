#include <model.hpp>
#include <ostream>
#include <printer.hpp>

namespace Reflector
{
	namespace
	{
		void print_annotation(std::ostream& stream, const Annotation& annotation)
		{
			stream << annotation.name << '(' << annotation.arguments << ") @ " << annotation.line << ':' << annotation.column;
			if (!annotation.metadata.empty())
			{
				stream << " metadata=[";
				for (std::size_t i = 0; i < annotation.metadata.size(); ++i)
				{
					const auto& metadata = annotation.metadata[i];
					if (i > 0)
					{
						stream << ", ";
					}
					if (!metadata.name.empty())
					{
						stream << metadata.name << '=';
					}
					stream << metadata.value;
				}
				stream << ']';
			}
		}

		const char* access_name(Access access)
		{
			switch (access)
			{
				case Access::Global: return "global";
				case Access::Private: return "private";
				case Access::Protected: return "protected";
				case Access::Public: return "public";
			}
			return "unknown";
		}

		void print_type_info(std::ostream& stream, const TypeInfo& type_info, const char* indent)
		{
			if (type_info.raw.empty())
			{
				return;
			}

			stream << indent << "  type info: " << type_info.qualified_name;
			if (!type_info.name.empty() && type_info.name != type_info.qualified_name)
			{
				stream << " name=" << type_info.name;
			}
			if (!type_info.namespaces.empty())
			{
				stream << " namespaces=";
				for (std::size_t i = 0; i < type_info.namespaces.size(); ++i)
				{
					if (i > 0)
					{
						stream << "::";
					}
					stream << type_info.namespaces[i];
				}
			}
			if (!type_info.template_arguments.empty())
			{
				stream << " template_args=" << type_info.template_arguments.size();
			}
			if (type_info.pointer_depth > 0)
			{
				stream << " pointer_depth=" << static_cast<unsigned int>(type_info.pointer_depth);
			}
			if (type_info.flags != TypeFlag_None)
			{
				stream << " flags=" << type_info.flags;
			}
			stream << '\n';
		}

		void print_property(std::ostream& stream, const Property& property, const char* indent)
		{
			stream << indent << "property " << property.name << '\n';
			stream << indent << "  full name: " << property.full_name << '\n';
			stream << indent << "  owner: " << (property.owner.empty() ? "<global>" : property.owner) << '\n';
			if (!property.attributes.empty())
			{
				stream << indent << "  attributes: " << property.attributes << '\n';
			}
			if (!property.engine_macros.empty())
			{
				stream << indent << "  engine macros: " << property.engine_macros << '\n';
			}
			stream << indent << "  type: " << property.type << '\n';
			print_type_info(stream, property.type_info, indent);
			stream << indent << "  access: " << access_name(property.access) << '\n';
			stream << indent << "  line: " << property.line << '\n';
			stream << indent << "  flags:";
			if (property.flags & PropertyFlag_Static)
			{
				stream << " static";
			}
			if (property.flags & PropertyFlag_Const)
			{
				stream << " const";
			}
			if (property.flags & PropertyFlag_Constexpr)
			{
				stream << " constexpr";
			}
			if (property.flags & PropertyFlag_Mutable)
			{
				stream << " mutable";
			}
			if (property.flags & PropertyFlag_Pointer)
			{
				stream << " pointer";
			}
			if (property.flags & PropertyFlag_Reference)
			{
				stream << " reference";
			}
			stream << '\n';
			if (!property.default_value.empty())
			{
				stream << indent << "  default: " << property.default_value << '\n';
			}
			stream << indent << "  declaration: " << property.declaration << '\n';
			stream << indent << "  annotation: ";
			print_annotation(stream, property.annotation);
			stream << '\n';
		}

		void print_function(std::ostream& stream, const Function& function, const char* indent)
		{
			stream << indent << "function " << function.name << '\n';
			stream << indent << "  full name: " << function.full_name << '\n';
			stream << indent << "  owner: " << (function.owner.empty() ? "<global>" : function.owner) << '\n';
			if (!function.template_prefix.empty())
			{
				stream << indent << "  template: " << function.template_prefix << '\n';
			}
			if (!function.attributes.empty())
			{
				stream << indent << "  attributes: " << function.attributes << '\n';
			}
			if (!function.engine_macros.empty())
			{
				stream << indent << "  engine macros: " << function.engine_macros << '\n';
			}
			stream << indent << "  return: " << (function.return_type.empty() ? "<constructor/destructor>" : function.return_type)
			       << '\n';
			print_type_info(stream, function.return_type_info, indent);
			stream << indent << "  access: " << access_name(function.access) << '\n';
			stream << indent << "  line: " << function.line << '\n';
			stream << indent << "  parameters: " << function.parsed_parameters.size() << '\n';
			for (const auto& parameter : function.parsed_parameters)
			{
				stream << indent << "    - " << (parameter.name.empty() ? "<unnamed>" : parameter.name) << ": " << parameter.type;
				if (!parameter.default_value.empty())
				{
					stream << " = " << parameter.default_value;
				}
				if (!parameter.type_info.name.empty())
				{
					stream << " [" << parameter.type_info.qualified_name << ']';
				}
				stream << '\n';
			}
			stream << indent << "  qualifiers: " << (function.qualifiers.empty() ? "<none>" : function.qualifiers) << '\n';
			stream << indent << "  flags:";
			if (function.flags & FunctionFlag_Static)
			{
				stream << " static";
			}
			if (function.flags & FunctionFlag_Virtual)
			{
				stream << " virtual";
			}
			if (function.flags & FunctionFlag_Const)
			{
				stream << " const";
			}
			if (function.flags & FunctionFlag_Constexpr)
			{
				stream << " constexpr";
			}
			if (function.flags & FunctionFlag_Inline)
			{
				stream << " inline";
			}
			if (function.flags & FunctionFlag_Noexcept)
			{
				stream << " noexcept";
			}
			if (function.flags & FunctionFlag_Override)
			{
				stream << " override";
			}
			if (function.flags & FunctionFlag_Final)
			{
				stream << " final";
			}
			if (function.flags & FunctionFlag_PureVirtual)
			{
				stream << " pure_virtual";
			}
			if (function.flags & FunctionFlag_Constructor)
			{
				stream << " constructor";
			}
			if (function.flags & FunctionFlag_Destructor)
			{
				stream << " destructor";
			}
			if (function.flags & FunctionFlag_Operator)
			{
				stream << " operator";
			}
			stream << '\n';
			stream << indent << "  declaration: " << function.declaration << '\n';
			stream << indent << "  annotation: ";
			print_annotation(stream, function.annotation);
			stream << '\n';
		}

	}// namespace

	void print(std::ostream& stream, const TranslationUnit& unit)
	{
		stream << "source: " << unit.source_name << '\n';
		stream << "types: " << unit.types.size() << '\n';

		for (const auto& type : unit.types)
		{
			stream << "  " << type.kind << ' ' << type.name << '\n';
			stream << "    full name: " << type.full_name << '\n';
			stream << "    scope: " << (type.scope.empty() ? "<global>" : type.scope) << '\n';
			if (!type.template_prefix.empty())
			{
				stream << "    template: " << type.template_prefix << '\n';
			}
			if (!type.attributes.empty())
			{
				stream << "    attributes: " << type.attributes << '\n';
			}
			if (!type.engine_macros.empty())
			{
				stream << "    engine macros: " << type.engine_macros << '\n';
			}
			stream << "    line: " << type.line << '\n';
			if (!type.bases.empty())
			{
				stream << "    bases: " << type.bases << '\n';
			}
			stream << "    annotation: ";
			print_annotation(stream, type.annotation);
			stream << '\n';
			if (!type.enum_values.empty())
			{
				stream << "    enum values: " << type.enum_values.size() << '\n';
				for (const auto& value : type.enum_values)
				{
					stream << "      - " << value << '\n';
				}
			}
			if (!type.nested_types.empty())
			{
				stream << "    nested types: " << type.nested_types.size() << '\n';
				for (const auto& nested_type : type.nested_types)
				{
					stream << "      " << nested_type.kind << ' ' << nested_type.name << " -> " << nested_type.full_name << '\n';
				}
			}
			stream << "    properties: " << type.properties.size() << '\n';

			for (const auto& property : type.properties)
			{
				print_property(stream, property, "    ");
			}

			stream << "    functions: " << type.functions.size() << '\n';
			for (const auto& function : type.functions)
			{
				print_function(stream, function, "    ");
			}
		}

		stream << "global properties: " << unit.properties.size() << '\n';
		for (const auto& property : unit.properties)
		{
			print_property(stream, property, "  ");
		}

		stream << "global functions: " << unit.functions.size() << '\n';
		for (const auto& function : unit.functions)
		{
			print_function(stream, function, "  ");
		}

		for (const auto& diagnostic : unit.diagnostics)
		{
			stream << "diagnostic: " << (diagnostic.severity == DiagnosticSeverity::Error ? "error" : "warning") << " ["
			       << diagnostic.line << ':' << diagnostic.column << "] " << diagnostic.message << '\n';
		}
	}

}// namespace Reflector
