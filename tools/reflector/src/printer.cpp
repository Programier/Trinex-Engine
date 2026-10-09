#include <initializer_list>
#include <iomanip>
#include <model.hpp>
#include <ostream>
#include <printer.hpp>
#include <utility>

namespace Reflector
{
	namespace
	{
		void print_type(std::ostream& stream, const TypeInfo& type)
		{
			if (type.flags & TypeInfo::Const)
				stream << "const ";
			if (type.flags & TypeInfo::Volatile)
				stream << "volatile ";
			stream << type.name;
			if (type.flags & TypeInfo::Template)
			{
				stream << '<';
				for (std::size_t i = 0; i < type.templates.size(); ++i)
				{
					if (i)
						stream << ", ";
					const auto& argument = type.templates[i];
					if (argument.flags & TypeInfo::TemplateArgument::Type)
						print_type(stream, argument.type);
					else
						stream << argument.value;
					if (argument.flags & TypeInfo::TemplateArgument::PackExpansion)
						stream << "...";
				}
				stream << '>';
			}
			// Function/member-pointer declarators already contain their pointer spelling.
			if ((type.flags & TypeInfo::Pointer) && type.name.find('*') == std::string::npos)
			{
				stream << '*';
				if (type.flags & TypeInfo::PointerConst)
					stream << " const";
				if (type.flags & TypeInfo::PointerVolatile)
					stream << " volatile";
			}
			if (type.flags & TypeInfo::Reference)
				stream << '&';
			if (type.flags & TypeInfo::RValueRef)
				stream << "&&";
		}

		void print_flags(std::ostream& stream, std::uint8_t flags,
		                 std::initializer_list<std::pair<std::uint8_t, std::string_view>> names)
		{
			stream << unsigned(flags) << " [";
			bool first = true;
			for (const auto& [flag, name] : names)
			{
				if (!(flags & flag))
					continue;
				if (!first)
					stream << ", ";
				stream << name;
				first = false;
			}
			if (first)
				stream << "none";
			stream << "]\n";
		}

		void print_type_details(std::ostream& stream, const TypeInfo& type, std::size_t depth, std::string_view label)
		{
			const std::string indent(depth * 2, ' ');
			stream << indent << label << ":\n";
			stream << indent << "  name: " << std::quoted(type.name) << '\n';
			stream << indent << "  flags: ";
			print_flags(stream, type.flags,
			            {{TypeInfo::Const, "const"},
			             {TypeInfo::Volatile, "volatile"},
			             {TypeInfo::Reference, "reference"},
			             {TypeInfo::RValueRef, "rvalue_ref"},
			             {TypeInfo::Template, "template"},
			             {TypeInfo::Pointer, "pointer"},
			             {TypeInfo::PointerConst, "pointer_const"},
			             {TypeInfo::PointerVolatile, "pointer_volatile"}});
			stream << indent << "  template_arguments: " << type.templates.size() << '\n';
			for (std::size_t i = 0; i < type.templates.size(); ++i)
			{
				const auto& argument = type.templates[i];
				stream << indent << "    [" << i << "] flags: ";
				using Argument = TypeInfo::TemplateArgument;
				print_flags(stream, argument.flags,
				            {{Argument::Type, "type"},
				             {Argument::Value, "value"},
				             {Argument::Template, "template"},
				             {Argument::PackExpansion, "pack_expansion"}});
				if (argument.flags & Argument::Type)
					print_type_details(stream, argument.type, depth + 3, "type");
				if (argument.flags & (Argument::Value | Argument::Template))
					stream << indent << "      value: " << std::quoted(argument.value) << '\n';
			}
		}

		const char* kind_name(ObjectKind kind)
		{
			switch (kind)
			{
				case ObjectKind::Object: return "object";
				case ObjectKind::Enum: return "enum";
				case ObjectKind::Scope: return "scope";
				case ObjectKind::Namespace: return "namespace";
				case ObjectKind::Struct: return "struct";
				case ObjectKind::Class: return "class";
				case ObjectKind::Module: return "module";
				case ObjectKind::Function: return "function";
				case ObjectKind::Property: return "property";
			}
			return "unknown";
		}
	}// namespace

	void print(std::ostream& stream, const Object* object, std::size_t depth)
	{
		if (object == nullptr)
			return;

		const std::string indent(depth * 2, ' ');
		stream << indent << kind_name(object->kind()) << ' ' << object->name;
		if (auto* property = dynamic_cast<const Property*>(object))
		{
			stream << ": ";
			print_type(stream, property->type);
			if (!property->value.empty())
				stream << " = " << property->value;
			stream << " flags=" << unsigned(property->flags);
		}
		else if (auto* function = dynamic_cast<const Function*>(object))
		{
			stream << '(';
			for (std::size_t i = 0; i < function->args.size(); ++i)
			{
				if (i)
					stream << ", ";
				const auto& argument = function->args[i];
				print_type(stream, argument.type);
				if (!argument.name.empty())
					stream << ' ' << argument.name;
				if (!argument.value.empty())
					stream << " = " << argument.value;
			}
			if (function->flags & Function::Variadic)
				stream << (function->args.empty() ? "..." : ", ...");
			stream << ") -> ";
			print_type(stream, function->type);
			stream << " flags=" << function->flags;
		}
		stream << " access=" << unsigned(object->access) << '\n';
		if (auto* property = dynamic_cast<const Property*>(object))
			print_type_details(stream, property->type, depth + 1, "type");
		else if (auto* function = dynamic_cast<const Function*>(object))
		{
			print_type_details(stream, function->type, depth + 1, "return");
			stream << indent << "  args: " << function->args.size() << '\n';
			for (std::size_t i = 0; i < function->args.size(); ++i)
			{
				const auto& argument = function->args[i];
				stream << indent << "    [" << i << "] name: " << std::quoted(argument.name) << '\n';
				stream << indent << "      default: ";
				if (argument.value.empty())
				{
					stream << "<none>\n";
				}
				else
				{
					stream << std::quoted(argument.value) << '\n';
				}
				print_type_details(stream, argument.type, depth + 3, "type");
			}
		}

		for (const auto& metadata : object->metadata)
		{
			stream << indent << "  metadata: " << metadata.name << '=' << metadata.value << '\n';
		}

		if (auto* enumeration = dynamic_cast<const Enum*>(object))
		{
			for (const auto& value : enumeration->values)
			{
				stream << indent << "  " << value.name << " = " << value.value << '\n';
				for (const auto& metadata : value.metadata)
					stream << indent << "    metadata: " << metadata.name << '=' << metadata.value << '\n';
			}
		}

		if (auto* structure = dynamic_cast<const Struct*>(object))
		{
			stream << indent << "  flags=" << unsigned(structure->flags) << '\n';
			for (const auto& base : structure->bases)
			{
				stream << indent << "  base: ";
				print_type(stream, base.type);
				stream << " access=" << unsigned(base.access) << " flags=" << unsigned(base.flags) << '\n';
			}
		}

		if (auto* scope = dynamic_cast<const Scope*>(object))
		{
			for (const auto& child : scope->objects)
			{
				if (child)
				{
					print(stream, child, depth + 1);
				}
			}
		}
	}
}// namespace Reflector
