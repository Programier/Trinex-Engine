#include <algorithm>
#include <fstream>
#include <generator.hpp>
#include <iostream>
#include <model.hpp>
#include <stream.hpp>
#include <string>

namespace Reflector
{
	namespace fs = std::filesystem;

	using NameOf              = WriteAs<struct Name>;
	using HexNameOf           = WriteAs<struct HexName>;
	using ResolvedNameOf      = WriteAs<struct ResolvedName>;
	using ResolvedPathOf      = WriteAs<struct ResolvedPath>;
	using ObjectIDOf          = WriteAs<struct ObjectID>;
	using ObjectPathOf        = WriteAs<struct ObjectPath>;
	using ReflectionOf        = WriteAs<struct Reflection>;
	using FunctionFlagsOf     = WriteAs<struct FunctionFlags>;
	using BaseClassesOf       = WriteAs<struct BaseClasses>;
	using FunctionPrototypeOf = WriteAs<struct FunctionPrototype>;

	struct Name
	{
		static void execute(CodeWriter* writer, const Object* object)
		{
			if (object == nullptr || object->is_a(ObjectKind::Module))
				return;

			if (const Function* function = dynamic_cast<const Function*>(object))
			{
				if (function->flags & Function::Constructor)
				{
					writer->write("$constructor");
				}
				else if (function->flags & Function::Operator)
				{
					writer->write("$operator");
				}
				else
				{
					writer->write(object->name);
				}
			}
			else
			{
				writer->write(object->name);
			}
		}
	};

	struct HexName
	{
		static void execute(CodeWriter* writer, const Object* object)
		{
			constexpr std::string_view digits = "0123456789abcdef";

			for (unsigned char ch : object->name)
			{
				writer->write(digits.substr(ch >> 4, 1));
				writer->write(digits.substr(ch & 0x0F, 1));
			}
		}
	};

	struct ResolvedName
	{
		static void execute(CodeWriter* writer, const Object* object)
		{
			if (object == nullptr || object->is_a(ObjectKind::Module))
				return;

			if (object->is_a(ObjectKind::Function))
			{
				writer->write("func_");
				writer->write(reinterpret_cast<std::uintptr_t>(object));
			}
			else if (object->is_a(ObjectKind::Property))
			{
				writer->write("prop_");
				writer->write(reinterpret_cast<std::uintptr_t>(object));
			}
			else
			{
				writer->write(object->name);

				if (object->is_a(ObjectKind::EnumStruct))
				{
					writer->write("::$0", static_cast<const EnumStruct*>(object)->enum_name);
				}
			}
		}
	};

	struct ResolvedPath
	{
		static void accessor(CodeWriter* writer, const Object* object, const Object* child = nullptr)
		{
			if (object == nullptr || object->is_a(ObjectKind::Module))
				return;

			const bool can_use_accessor = child == nullptr || !child->is_a(ObjectKind::Namespace);

			if (object->is_a(ObjectKind::Scope) && can_use_accessor)
			{
				writer->write("Accessor_$0", reinterpret_cast<std::uintptr_t>(object));
				return;
			}

			accessor(writer, object->owner, object);
			writer->write("::$0", ResolvedNameOf(object));
		}

		static void execute(CodeWriter* writer, const Object* object)
		{
			if (object == nullptr || object->is_a(ObjectKind::Module))
				return;

			accessor(writer, object->owner, object);
			writer->write("::$0", ResolvedNameOf(object));
		}
	};

	struct ObjectID
	{
		static bool execute(CodeWriter* writer, const Object* object)
		{
			if (object == nullptr)
				return false;

			if (execute(writer, object->owner))
			{
				writer->write("_");
			}

			if (object->is_a(ObjectKind::Module))
			{
				std::string path = object->name;

				for (char& character : path)
				{
					if (character == '/' || character == '\\' || character == '.')
						character = '_';
				}

				writer->write(path);
			}
			else
			{
				writer->write(object->name);
			}

			return true;
		}
	};

	struct ObjectPath
	{
		static bool execute_list(CodeWriter* writer, const Object* object)
		{
			if (object == nullptr || object->is_a(ObjectKind::Module))
				return false;

			if (execute_list(writer, object->owner))
			{
				writer->write(", ");
			}

			writer->write("\"$0\"", object->name);
			return true;
		}

		static void execute(CodeWriter* writer, const Object* object)
		{
			writer->write("{");
			execute_list(writer, object);
			writer->write("}");
		}
	};

	struct Reflection
	{
		static void execute(CodeWriter* writer, const Object* object)
		{
			if (object == nullptr || object->is_a(ObjectKind::Module))
			{
				writer->write("Object::root()");
				return;
			}

			if (object->is_a(ObjectKind::Class))
			{
				writer->write("type_of<$0>()", object);
			}
			else
			{
				if (object->is_a(ObjectKind::Module))
				{
					writer->write("Object::root()");
				}
				else
				{
					writer->write("Scope::create($0)", ObjectPathOf(object));
				}
			}
		}
	};

	struct FunctionFlags
	{
		static void execute(CodeWriter* writer, const Object* object)
		{
			if (object == nullptr || !object->is_a(ObjectKind::Function))
			{
				writer->write("0");
				return;
			}

			const Function* function = static_cast<const Function*>(object);
			writer->write("Func::flags");

			if (function->flags & Function::Final)
				writer->write(" | Function::IsFinal");

			if (function->flags & Function::Constructor)
				writer->write(" | Function::IsConstructor");

			if (function->flags & Function::Destructor)
				writer->write(" | Function::IsDestructor");

			if (function->flags & Function::Operator)
				writer->write(" | Function::IsOperator");
		}
	};

	struct BaseClasses
	{
		static void execute(CodeWriter* writer, const Object* object)
		{
			if (object == nullptr || !object->is_a(ObjectKind::Class))
			{
				return;
			}

			const Class* instance = static_cast<const Class*>(object);

			for (std::size_t i = 0; i < instance->bases.size(); ++i)
			{
				if (i != 0)
				{
					writer->write(", ");
				}

				writer->write(instance->bases[i].type.name);
			}
		}
	};

	struct FunctionPrototype
	{
		static void execute(CodeWriter* writer, const TypeInfo& type)
		{
			if (type.flags & TypeInfo::Const)
			{
				writer->write("const ");
			}

			if (type.flags & TypeInfo::Volatile)
			{
				writer->write("volatile ");
			}

			writer->write(type.name);

			if (type.flags & TypeInfo::Template)
			{
				writer->write("<");

				for (std::size_t index = 0; index < type.templates.size(); ++index)
				{
					if (index != 0)
					{
						writer->write(", ");
					}

					const TypeInfo::TemplateArgument& argument = type.templates[index];

					if (argument.flags & TypeInfo::TemplateArgument::Type)
					{
						execute(writer, argument.type);
					}
					else
					{
						writer->write(argument.value);
					}

					if (argument.flags & TypeInfo::TemplateArgument::PackExpansion)
					{
						writer->write("...");
					}
				}

				writer->write(">");
			}

			if ((type.flags & TypeInfo::Pointer) && type.name.find('*') == std::string::npos)
			{
				writer->write("*");

				if (type.flags & TypeInfo::PointerConst)
				{
					writer->write(" const");
				}

				if (type.flags & TypeInfo::PointerVolatile)
				{
					writer->write(" volatile");
				}
			}

			if (type.flags & TypeInfo::Reference)
			{
				writer->write("&");
			}

			if (type.flags & TypeInfo::RValueRef)
			{
				writer->write("&&");
			}
		}

		static void execute(CodeWriter* writer, const Object* object)
		{
			if (object == nullptr || !object->is_a(ObjectKind::Function))
			{
				return;
			}

			const Function* function = static_cast<const Function*>(object);

			execute(writer, function->type);

			if (function->is_method())
			{
				writer->write(" ($0::*)(", ResolvedPathOf(function->owner));
			}
			else
			{
				writer->write(" (*)(");
			}

			for (std::size_t index = 0; index < function->args.size(); ++index)
			{
				if (index != 0)
				{
					writer->write(", ");
				}

				execute(writer, function->args[index].type);
			}

			if (function->flags & Function::Variadic)
			{
				if (!function->args.empty())
				{
					writer->write(", ");
				}

				writer->write("...");
			}

			writer->write(")");

			if (function->flags & Function::Const)
			{
				writer->write(" const");
			}
		}
	};

	static bool is_object_empty(const Object* object)
	{
		if (object == nullptr)
			return true;

		switch (object->kind())
		{
			case ObjectKind::Enum:
			case ObjectKind::EnumStruct:
			case ObjectKind::Class:
			case ObjectKind::Function:
			case ObjectKind::Property: return false;

			case ObjectKind::Object: return true;

			case ObjectKind::Scope:
			case ObjectKind::Namespace:
			case ObjectKind::Module:
			{
				auto scope = dynamic_cast<const Scope*>(object);

				for (Object* child : scope->objects)
				{
					if (!is_object_empty(child))
					{
						return false;
					}
				}

				return true;
			}

			default: return true;
		}
	}

	static void write_generated_header(CodeWriter* writer)
	{
		writer->write("// ============================================================================\n"
		              "// Trinex Engine - Auto-generated File\n"
		              "// Generated by Trinex Reflector\n"
		              "//\n"
		              "// DO NOT EDIT! All changes will be overwritten during code generation.\n"
		              "// ============================================================================\n\n");
	}

	static void generate_accessor(CodeWriter* writer, const Scope* object)
	{
		if (object->is_a(ObjectKind::Module))
			return;

		const bool is_class = object->is_a(ObjectKind::Class);

		writer->line("// Accessor for $0", object);
		if (is_class)
		{
			writer->line("template<>");
			writer->line("struct Accessor<$0>", ResolvedPathOf(object));
		}
		else
		{
			writer->line("struct Accessor_$0", reinterpret_cast<std::uintptr_t>(object));
		}

		writer->line("{");
		{
			auto scope = writer->scope();

			for (Object* child : object->objects)
			{
				if (child->is_a(ObjectKind::Property))
				{
					writer->write("static constexpr auto $0 = &", ResolvedNameOf(child));
					writer->line("$0::$1;", ResolvedPathOf(object), child->name);
				}
				else if (auto function = dynamic_cast<Function*>(child))
				{
					writer->write("static constexpr auto $0 = ", ResolvedNameOf(child));

					if (function->flags & Function::Constructor)
					{
						writer->write("+[](void* mem");

						for (std::size_t index = 0; index < function->args.size(); ++index)
						{
							writer->write(", ");
							FunctionPrototype::execute(writer, function->args[index].type);
							writer->write(" arg$0", index);
						}

						writer->line(")");
						writer->line("{");
						{
							auto lambda_scope = writer->scope();
							writer->write("::new (mem) $0(", ResolvedPathOf(object));

							for (std::size_t index = 0; index < function->args.size(); ++index)
							{
								if (index != 0)
								{
									writer->write(", ");
								}

								writer->write("etl::forward<");
								FunctionPrototype::execute(writer, function->args[index].type);
								writer->write(">(arg$0)", index);
							}

							writer->line(");");
						}
						writer->line("};\n");
					}
					else
					{
						writer->write("static_cast<$0>", FunctionPrototypeOf(child));
						writer->line("(&$0::$1);", ResolvedPathOf(object), child->name);
					}
				}
				else if (child->is_a(ObjectKind::Class) || child->is_a(ObjectKind::Enum))
				{
					writer->write("using $0 = ", child->name);
					writer->line("$0::$1;", ResolvedPathOf(object), child->name);
				}
			}
		}
		writer->line("};\n");

		if (is_class)
		{
			writer->write("using Accessor_$0", reinterpret_cast<std::uintptr_t>(object));
			writer->line(" = Accessor<$0>;\n", ResolvedPathOf(object));
		}
	}

	static void generate_function_sets(CodeWriter* writer, const Scope* object)
	{
		static thread_local std::vector<std::string_view> storage(128, "");

		std::uintptr_t id = reinterpret_cast<uintptr_t>(object);
		std::size_t sets  = 0;

		for (Object* child : object->objects)
		{
			if (!child->is_a(ObjectKind::Function))
				continue;

			if (std::find(storage.begin(), storage.begin() + sets, child->name) != storage.begin() + sets)
				continue;

			if (sets == storage.size())
			{
				storage.resize(storage.size() * 2);
			}

			storage[sets++] = child->name;

			writer->line("static FunctionSet* function_set_$0_$1()\n{", id, HexNameOf(child));
			{
				auto scope = writer->scope();
				writer->line("static FunctionSet set(\"$0\", $1);", NameOf(child), ReflectionOf(object));
				writer->line("return &set;");
			}
			writer->line("}\n");
		}
	}

	static void generate_enum(CodeWriter* writer, const Enum* object)
	{
		writer->line("template<>");
		writer->line("trinex_export Enum* EnumOf<$0>::resolve()", ResolvedPathOf(object));
		writer->line("{");
		{
			auto scope = writer->scope();

			writer->line("static Ref<Enum> reflection = []()");
			writer->line("{");
			{
				auto scope = writer->scope();

				writer->line("auto underlying = retain_ref(type_of<etl::underlying_type_t<$0>>());", ResolvedPathOf(object));
				writer->line("static Native::EnumVTable<$0, Enum::VTable> vtable;\n", ResolvedPathOf(object));

				writer->line("Array<EnumValue> values = {");
				{
					auto scope = writer->scope();
					for (auto& value : object->values)
					{
						writer->line("{\"$1\", static_cast<i64>($0::$1)},", ResolvedPathOf(object), value.name);
					}
				}
				writer->line("};\n");
				writer->line("return Ref<Enum>::make(\"$0\", etl::move(underlying), etl::move(values), 0, &vtable, $1);",
				             object->name, ReflectionOf(object->owner));
			}
			writer->line("}();\n");
			writer->line("return reflection.value();");
		}
		writer->line("}\n");

		writer->line("template<>");
		writer->line("trinex_export Enum* EnumOf<$0::$1>::resolve()", ResolvedPathOf(object->owner), object->name);
		writer->line("{");
		{
			auto scope = writer->scope();
			writer->line("return enum_of<$0>();", ResolvedPathOf(object));
		}
		writer->line("}\n");

		writer->line("template<>");
		writer->line("trinex_export Type* TypeOf<$0>::resolve()", ResolvedPathOf(object));
		writer->line("{");
		{
			auto scope = writer->scope();
			writer->line("return enum_of<$0>();", ResolvedPathOf(object));
		}
		writer->line("}\n");

		writer->line("template<>");
		writer->line("trinex_export Type* TypeOf<$0::$1>::resolve()", ResolvedPathOf(object->owner), object->name);
		writer->line("{");
		{
			auto scope = writer->scope();
			writer->line("return enum_of<$0>();", ResolvedPathOf(object));
		}
		writer->line("}\n");
	}

	static void generate_property(CodeWriter* writer, const Property* object)
	{
		writer->line("template<>");
		writer->line("trinex_export Property* property_of<$0>()", ResolvedPathOf(object));
		writer->line("{");
		{
			auto scope = writer->scope();
			writer->line("using Prop = Native::MemberProperty<$0, Property>;\n", ResolvedPathOf(object));

			writer->line("static Prop prop(\"$0\", type_of<Prop::Type>(), 0, $1);", object->name, ReflectionOf(object->owner));
			writer->line("return &prop;");
		}
		writer->line("}\n");
	}

	static void generate_function(CodeWriter* writer, const Function* object)
	{
		writer->line("template<>");
		writer->line("trinex_export Function* function_of<$0>()", ResolvedPathOf(object));
		writer->line("{");
		{
			auto scope = writer->scope();
			writer->line("using Func = Native::Function<$0, NativeFunction>;\n", ResolvedPathOf(object));

			writer->line("static Ref<Function> func = adopt_ref([]");
			writer->line("{");
			{
				auto scope = writer->scope();

				const Object* owner  = object->owner;
				const bool is_method = owner && owner->is_a(ObjectKind::Class) && !(object->flags & Function::Static);

				writer->line("Ref<Property> result;\n");

				writer->line("if constexpr (!etl::is_void_v<Func::ReturnType>)\n{");
				{
					auto scope = writer->scope();
					writer->line("using Result = Native::OffsetProperty<Func::return_offset, Property>;");
					writer->line("result = Ref<Result>::make(\"return\", type_of<Func::ReturnType>());");
				}
				writer->line("}\n");

				const std::size_t arg_offset = is_method ? 1 : 0;

				if (is_method)
				{
					writer->line("using Arg0 = Native::OffsetProperty<Func::argument_offset<0>(), Property>;");
					writer->line("auto arg0 = Ref<Arg0>::make(\"this\", type_of<Func::ArgumentType<0>>(), "
					             "Property::IsThis);\n");
				}

				for (std::size_t index = 0; index < object->args.size(); ++index)
				{
					std::size_t arg                    = arg_offset + index;
					const Function::Argument& argument = object->args[index];
					const std::string name             = argument.name.empty() ? "arg" + std::to_string(index) : argument.name;

					writer->line("using Arg$0 = Native::OffsetProperty<Func::argument_offset<$0>(), Property>;", arg);
					writer->line("auto arg$0 = Ref<Arg$0>::make(\"$1\", type_of<Func::ArgumentType<$0>>());\n", arg, name);
				}

				writer->line("return trx_new NativeFunction(");
				{
					auto scope = writer->scope();
					writer->line("\"$0\",", object->name);
					writer->line("reinterpret_cast<NativeFunction::Address>(Func::address),");
					writer->line("Func::invoker<const Function*>,");
					writer->line("result,");
					writer->write("{");

					for (std::size_t index = 0, count = arg_offset + object->args.size(); index < count; ++index)
					{
						if (index != 0)
							writer->write(", ");
						writer->write("arg$0", index);
					}
					std::uintptr_t id = reinterpret_cast<std::uintptr_t>(object->owner);
					writer->line("},\n$0,\nfunction_set_$1_$2()", FunctionFlagsOf(object), id, HexNameOf(object));
				}
				writer->line(");");
			}

			writer->line("}());\n");
			writer->line("return func.value();");
		}
		writer->line("}\n");
	}

	static void generate_class(CodeWriter* writer, const Class* object)
	{
		writer->line("template<>");
		writer->line("trinex_export Struct* StructOf<$0>::resolve()", ResolvedPathOf(object));
		writer->line("{");
		{
			auto indent = writer->scope();
			writer->line("using This = $0;\n", ResolvedPathOf(object));

			writer->line("constexpr bool is_ref_counted = etl::is_base_of_v<::Trinex::RefCounted, This>;");
			writer->line("using StructVTable = Native::StructVTable<This, Struct::VTable>;");
			writer->line("using ClassVTable = Native::ClassVTable<This, Class::VTable>;\n");

			writer->line("using VTable = etl::conditional_t<is_ref_counted, ClassVTable, StructVTable>;");
			writer->line("using Result = etl::conditional_t<is_ref_counted, Class, Struct>;\n");

			writer->line("static Ref<Result> result = []");
			writer->line("{");
			{
				auto indent = writer->scope();

				writer->line("auto bases = Native::gather_reflections<Array<Ref<Struct>>, This::Bases>([]<typename T>");
				writer->line("{");
				{
					auto scope = writer->scope();
					writer->line("return retain_ref(struct_of<T>());");
				}
				writer->line("});\n");

				writer->line("static VTable vtable;");
				writer->line("return Ref<Result>::make(\"$0\", sizeof(This), alignof(This), Type::IsNative, etl::move(bases), "
				             "&vtable, $1);",
				             object->name, ReflectionOf(object->owner));
			}
			writer->line("}();\n");

			writer->line("return result.value();");
		}
		writer->line("}\n");

		writer->line("template<>");
		writer->line("trinex_export Class* ClassOf<$0>::resolve()", ResolvedPathOf(object));
		writer->line("{");
		{
			auto indent = writer->scope();
			writer->line("if constexpr (etl::is_base_of_v<Trinex::RefCounted, $0>)", ResolvedPathOf(object));
			writer->line("{");
			{
				auto indent = writer->scope();
				writer->line("return static_cast<Class*>(struct_of<$0>());", ResolvedPathOf(object));
			}
			writer->line("}");
			writer->line("else");
			writer->line("{");
			{
				auto indent = writer->scope();
				writer->line("return nullptr;");
			}
			writer->line("}");
		}
		writer->line("}\n");

		writer->line("template<>");
		writer->line("trinex_export Type* TypeOf<$0>::resolve()", object);
		writer->line("{");
		{
			auto indent = writer->scope();
			writer->line("return struct_of<$0>();", object);
		}
		writer->line("}\n");
	}

	static void generate_reflection(CodeWriter* writer, const Scope* scope)
	{
		generate_function_sets(writer, scope);
		generate_accessor(writer, scope);

		for (const Object* object : scope->objects)
		{
			if (object->is_a(ObjectKind::Enum))
			{
				generate_enum(writer, static_cast<const Enum*>(object));
			}
			else if (object->is_a(ObjectKind::Property))
			{
				generate_property(writer, static_cast<const Property*>(object));
			}
			else if (object->is_a(ObjectKind::Function))
			{
				generate_function(writer, static_cast<const Function*>(object));
			}
			else if (object->is_a(ObjectKind::Class))
			{
				generate_class(writer, static_cast<const Class*>(object));
			}

			if (object->is_a(ObjectKind::Scope))
			{
				generate_reflection(writer, static_cast<const Scope*>(object));
			}
		}
	}

	static void generate_reflection_initialization(CodeWriter* writer, const Scope* scope)
	{
		for (const Object* object : scope->objects)
		{
			switch (object->kind())
			{
				case ObjectKind::Class: writer->line("type_of<$0>();", ResolvedPathOf(object)); break;
				case ObjectKind::Property: writer->line("property_of<$0>();", ResolvedPathOf(object)); break;
				case ObjectKind::Function: writer->line("function_of<$0>();", ResolvedPathOf(object)); break;
				case ObjectKind::Enum: writer->line("enum_of<$0>();", ResolvedPathOf(object)); break;
				case ObjectKind::EnumStruct: writer->line("enum_of<$0>();", ResolvedPathOf(object)); break;

				default: break;
			}

			if (object->is_a(ObjectKind::Scope))
			{
				generate_reflection_initialization(writer, static_cast<const Scope*>(object));
			}
		}
	}

	static void generate_reflection_body(CodeWriter* writer, const Module* module, const Class* instance)
	{
		if (instance->body_line == 0)
			return;

		writer->line("#define trinex_reflection_$0_$1 \\", ObjectIDOf(module), instance->body_line);
		{
			auto scope = writer->scope();

			writer->line("private: \\");
			{
				auto scope = writer->scope();
				writer->line("friend class ::Trinex::Refl::Binder<$0>; \\", instance->name);
				writer->write("[[no_unique_address]] ::Trinex::Refl::Binder<$0> ", instance->name);
				writer->line("m_triner_reflection_binder_$0 = this; \\", instance->body_line);
			}

			writer->line("public: \\");
			{
				auto scope = writer->scope();

				writer->line("static constexpr inline const bool is_reflected = true; \\");
				writer->line("using Bases = ::Trinex::TypeList<$0>; \\", BaseClassesOf(instance));
				writer->line("using This = $0; \\", instance->name);
				writer->line("friend struct ::Trinex::Refl::Accessor<$0>", instance->name);
			}
		}
	}

	static void generate_reflection_bodies(CodeWriter* writer, const Module* module, const Scope* scope)
	{
		for (const Object* object : scope->objects)
		{
			if (object->is_a(ObjectKind::Class))
			{
				const Class* instance = static_cast<const Class*>(object);
				generate_reflection_body(writer, module, instance);
			}

			if (object->is_a(ObjectKind::Scope))
			{
				generate_reflection_bodies(writer, module, static_cast<const Scope*>(object));
			}
		}
	}

	static bool generate_header(CodeWriter* writer, const Module* module)
	{
		write_generated_header(writer);

		if (is_object_empty(module))
			return true;

		writer->line("#include <Core/reflection/accessor.hpp>");
		writer->line("#include <Core/reflection/binder.hpp>\n");

		writer->line("#ifdef trinex_reflection_file_id");
		writer->line("#undef trinex_reflection_file_id");
		writer->line("#endif\n");

		writer->line("#define trinex_reflection_file_id trinex_reflection_$0", ObjectIDOf(module));

		generate_reflection_bodies(writer, module, module);
		return true;
	}

	static bool generate_source(CodeWriter* writer, const Module* module)
	{
		write_generated_header(writer);

		if (is_object_empty(module))
			return true;

		writer->line("#include <$0>", module->name);
		writer->line("#include <Core/etl/type_traits.hpp>");
		writer->line("#include <Core/reflection/of.hpp>");
		writer->line("#include <Core/reflection/enum.hpp>");
		writer->line("#include <Core/reflection/scope.hpp>");
		writer->line("#include <Core/reflection/class.hpp>");
		writer->line("#include <Core/reflection/function_set.hpp>");
		writer->line("#include <Core/reflection/native_function.hpp>");
		writer->line("#include <Core/reflection/native.hpp>");
		writer->line();

		writer->line("namespace Trinex");
		writer->line("{");
		{
			auto scope = writer->scope();
			writer->line("class RefCounted;");
		}
		writer->line("}");
		writer->line();

		writer->line("namespace Trinex::Refl");
		writer->line("{");
		{
			auto indent = writer->scope();
			generate_reflection(writer, module);

			writer->line("trinex_on_reflection_init()");
			writer->line("{");
			{
				auto initialization_indent = writer->scope();
				generate_reflection_initialization(writer, module);
			}
			writer->line("}");
		}
		writer->line("}");
		return true;
	}

	static bool save_generated_file(const fs::path& path, CodeWriter* writer)
	{
		const fs::path parent = path.parent_path();

		if (!parent.empty())
		{
			std::error_code ec;
			fs::create_directories(parent, ec);

			if (ec)
			{
				std::cerr << "Error: Failed to create directory '" << parent << "': " << ec.message() << '\n';
				return false;
			}
		}

		std::ofstream file(path, std::ios::binary | std::ios::trunc);

		if (!file)
		{
			std::cerr << "Error: Failed to open generated file: " << path << '\n';
			return false;
		}

		auto content = writer->text();
		file.write(content.data(), content.size());
		file.close();

		if (!file)
		{
			std::cerr << "Error: Failed to write generated file: " << path << '\n';
			return false;
		}

		return true;
	}

	bool generate_header(fs::path path, const Module* module)
	{
		if (is_object_empty(module))
			return true;

		CodeWriter header;

		if (!generate_header(&header, module))
		{
			return false;
		}

		return save_generated_file(path, &header);
	}

	bool generate_source(fs::path path, const Module* module)
	{
		CodeWriter source;

		if (!generate_source(&source, module))
		{
			return false;
		}

		return save_generated_file(path, &source);
	}
} // namespace Reflector
