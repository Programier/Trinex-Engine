#include <Core/engine_loading_controllers.hpp>
#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Core/memory.hpp>
#include <Core/reflection/class.hpp>
#include <Graphics/visual_material_graph.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>

namespace Engine::VisualMaterialGraph
{
	static ScriptFunction s_node_compile_output;

	trinex_implement_class(Engine::VisualMaterialGraph::Node, Refl::Class::IsScriptable)
	{
		auto r = ScriptClassRegistrar::reference_class(static_class_instance());

		s_node_compile_output =
				r.method("Expression compile(InputPin@ pin, Compiler& compiler)", trinex_scoped_void_method(This, compile));

		r.method("InputPin@ new_input(const string& name, ShaderParameterType type) final", &This::new_input);
		r.method("OutputPin@ new_output(const string& name, ShaderParameterType type) final", &This::new_output);

		ScriptEngine::on_terminate.push([]() { s_node_compile_output.release(); });
	}

	trinex_implement_class(Engine::VisualMaterialGraph::MaterialRoot, 0) {}

	const char* Expression::s_swizzle[4] = {".x", ".y", ".z", ".w"};

	Expression Expression::static_zero(ShaderParameterType type)
	{
		switch (type)
		{
			case ShaderParameterType::Bool:
				return Expression(type, "false");
			case ShaderParameterType::Bool2:
				return Expression(type, "bool2(false, false)");
			case ShaderParameterType::Bool3:
				return Expression(type, "bool3(false, false, false)");
			case ShaderParameterType::Bool4:
				return Expression(type, "bool4(false, false, false)");

			case ShaderParameterType::Int:
				return Expression(type, "int(0)");
			case ShaderParameterType::Int2:
				return Expression(type, "int2(0, 0)");
			case ShaderParameterType::Int3:
				return Expression(type, "int3(0, 0, 0)");
			case ShaderParameterType::Int4:
				return Expression(type, "int4(0, 0, 0, 0)");

			case ShaderParameterType::UInt:
				return Expression(type, "uint(0)");
			case ShaderParameterType::UInt2:
				return Expression(type, "uint2(0, 0)");
			case ShaderParameterType::UInt3:
				return Expression(type, "uint3(0, 0, 0)");
			case ShaderParameterType::UInt4:
				return Expression(type, "uint4(0, 0, 0, 0)");

			case ShaderParameterType::Float:
				return Expression(type, "float(0.f)");
			case ShaderParameterType::Float2:
				return Expression(type, "float2(0.f, 0.f)");
			case ShaderParameterType::Float3:
				return Expression(type, "float3(0.f, 0.f, 0.f)");
			case ShaderParameterType::Float4:
				return Expression(type, "float4(0.f, 0.f, 0.f, 0.f)");
			case ShaderParameterType::Float3x3:
				return Expression(type, "float3x3(0.f)");
			case ShaderParameterType::Float4x4:
				return Expression(type, "float4x4(0.f)");
			default:
				throw EngineException("Unsupported shader parameter type!");
		}
	}

	Expression Expression::static_half(ShaderParameterType type)
	{
		switch (type)
		{
			case ShaderParameterType::Bool:
				return Expression(type, "false");
			case ShaderParameterType::Bool2:
				return Expression(type, "bool2(false, false)");
			case ShaderParameterType::Bool3:
				return Expression(type, "bool3(false, false, false)");
			case ShaderParameterType::Bool4:
				return Expression(type, "bool4(false, false, false)");

			case ShaderParameterType::Int:
				return Expression(type, "int(0)");
			case ShaderParameterType::Int2:
				return Expression(type, "int2(0, 0)");
			case ShaderParameterType::Int3:
				return Expression(type, "int3(0, 0, 0)");
			case ShaderParameterType::Int4:
				return Expression(type, "int4(0, 0, 0, 0)");

			case ShaderParameterType::UInt:
				return Expression(type, "uint(0)");
			case ShaderParameterType::UInt2:
				return Expression(type, "uint2(0, 0)");
			case ShaderParameterType::UInt3:
				return Expression(type, "uint3(0, 0, 0)");
			case ShaderParameterType::UInt4:
				return Expression(type, "uint4(0, 0, 0, 0)");

			case ShaderParameterType::Float:
				return Expression(type, "float(0.5f)");
			case ShaderParameterType::Float2:
				return Expression(type, "float2(0.5f, 0.5f)");
			case ShaderParameterType::Float3:
				return Expression(type, "float3(0.5f, 0.5f, 0.5f)");
			case ShaderParameterType::Float4:
				return Expression(type, "float4(0.5f, 0.5f, 0.5f, 0.5f)");
			case ShaderParameterType::Float3x3:
				return Expression(type, "float3x3(0.5f)");
			case ShaderParameterType::Float4x4:
				return Expression(type, "float4x4(0.5f)");
			default:
				throw EngineException("Unsupported shader parameter type!");
		}
	}

	Expression Expression::static_one(ShaderParameterType type)
	{
		switch (type)
		{
			case ShaderParameterType::Bool:
				return Expression(type, "true");
			case ShaderParameterType::Bool2:
				return Expression(type, "bool2(true, true)");
			case ShaderParameterType::Bool3:
				return Expression(type, "bool3(true, true, true)");
			case ShaderParameterType::Bool4:
				return Expression(type, "bool4(true, true, true)");

			case ShaderParameterType::Int:
				return Expression(type, "int(1)");
			case ShaderParameterType::Int2:
				return Expression(type, "int2(1, 1)");
			case ShaderParameterType::Int3:
				return Expression(type, "int3(1, 1, 1)");
			case ShaderParameterType::Int4:
				return Expression(type, "int4(1, 1, 1, 1)");

			case ShaderParameterType::UInt:
				return Expression(type, "uint(1)");
			case ShaderParameterType::UInt2:
				return Expression(type, "uint2(1, 1)");
			case ShaderParameterType::UInt3:
				return Expression(type, "uint3(1, 1, 1)");
			case ShaderParameterType::UInt4:
				return Expression(type, "uint4(1, 1, 1, 1)");

			case ShaderParameterType::Float:
				return Expression(type, "float(1.f)");
			case ShaderParameterType::Float2:
				return Expression(type, "float2(1.f, 1.f)");
			case ShaderParameterType::Float3:
				return Expression(type, "float3(1.f, 1.f, 1.f)");
			case ShaderParameterType::Float4:
				return Expression(type, "float4(1.f, 1.f, 1.f, 1.f)");
			case ShaderParameterType::Float3x3:
				return Expression(type, "float3x3(1.f)");
			case ShaderParameterType::Float4x4:
				return Expression(type, "float4x4(1.f)");
			default:
				throw EngineException("Unsupported shader parameter type!");
		}
	}

	ShaderParameterType Expression::static_component_type_of(ShaderParameterType type)
	{
		switch (type)
		{
			case ShaderParameterType::Bool2:
			case ShaderParameterType::Bool3:
			case ShaderParameterType::Bool4:
				return ShaderParameterType::Bool;

			case ShaderParameterType::Int2:
			case ShaderParameterType::Int3:
			case ShaderParameterType::Int4:
				return ShaderParameterType::Int;

			case ShaderParameterType::UInt2:
			case ShaderParameterType::UInt3:
			case ShaderParameterType::UInt4:
				return ShaderParameterType::UInt;

			case ShaderParameterType::Float2:
			case ShaderParameterType::Float3:
			case ShaderParameterType::Float4:
			case ShaderParameterType::Float3x3:
			case ShaderParameterType::Float4x4:
				return ShaderParameterType::Float;

			default:
				return type;
		}
	}

	ShaderParameterType Expression::static_resolve(ShaderParameterType type1, ShaderParameterType type2)
	{
		if (type1 == type2)
			return type1;

		if ((type1.is_scalar() || type1.is_vector()) && (type2.is_scalar() || type2.is_vector()))
		{
			auto type_component1 = static_component_type_of(type1);
			auto type_component2 = static_component_type_of(type2);
			byte result_len      = glm::max(type1.vector_length(), type2.vector_length());

			if (type_component1 == ShaderParameterType::Float || type_component2 == ShaderParameterType::Float)
				return ShaderParameterType(ShaderParameterType::Float).make_vector(result_len);

			if (type_component1 == ShaderParameterType::Bool || type_component2 == ShaderParameterType::Bool)
				return ShaderParameterType(ShaderParameterType::Bool).make_vector(result_len);

			if (type_component1 == ShaderParameterType::UInt || type_component2 == ShaderParameterType::UInt)
				return ShaderParameterType(ShaderParameterType::UInt).make_vector(result_len);

			return ShaderParameterType(ShaderParameterType::Int).make_vector(result_len);
		}

		return ShaderParameterType();
	}

	String Expression::static_typename_of(ShaderParameterType type)
	{
		// clang-format off
		switch (type)
		{
			case ShaderParameterType::Bool: return "bool";
			case ShaderParameterType::Bool2: return "bool2";
			case ShaderParameterType::Bool3: return "bool3";
			case ShaderParameterType::Bool4: return "bool4";
			case ShaderParameterType::Int: return "int";
			case ShaderParameterType::Int2: return "int2";
			case ShaderParameterType::Int3: return "int3";
			case ShaderParameterType::Int4: return "int4";
			case ShaderParameterType::UInt: return "uint";
			case ShaderParameterType::UInt2: return "uint2";
			case ShaderParameterType::UInt3: return "uint3";
			case ShaderParameterType::UInt4: return "uint4";
			case ShaderParameterType::Float: return "float";
			case ShaderParameterType::Float2: return "float2";
			case ShaderParameterType::Float3: return "float3";
			case ShaderParameterType::Float4: return "float4";
			case ShaderParameterType::Float3x3: return "float3x3";
			case ShaderParameterType::Float4x4: return "float4x4";
			case ShaderParameterType::Sampler: return "SamplerState";
			case ShaderParameterType::Sampler2D: return "Sampler2D";
			case ShaderParameterType::Texture2D: return "Texture2D";
			default:
				break;
		}
		// clang-format on
		throw EngineException("Unsupported type!");
	}

	Expression Expression::static_convert(const Expression& expression, ShaderParameterType type)
	{
		if (expression.type == type || expression.value.empty())
			return expression;

		if ((type.is_vector() || type.is_scalar()) && (expression.type.is_vector() || expression.type.is_scalar()))
		{
			size_t src_components = expression.type.vector_length();
			size_t dst_components = type.vector_length();

			const auto src_component_type = static_component_type_of(expression.type);
			const auto dst_component_type = static_component_type_of(type);

			String result;

			const bool need_wrap = !(src_component_type == dst_component_type && src_components > dst_components);

			if (need_wrap)
				result = Strings::format("{}({}", static_typename_of(type), expression.value);
			else
				result = expression.value;


			if (src_components != 1)
			{
				if (dst_components < src_components)
				{
					StringView swizzle = StringView("xyzw").substr(0, dst_components);
					result.push_back('.');
					result += swizzle;
				}

				if (dst_components > src_components)
				{
					size_t push_count = dst_components - src_components;
					Expression zero   = static_zero(dst_component_type);

					while (push_count > 0)
					{
						result += ", ";
						result += zero.value;
						--push_count;
					}
				}
			}

			if (need_wrap)
				result.push_back(')');

			return Expression(type, result);
		}


		throw EngineException("Unsupported expression type for cast");
	}

	Expression Expression::x() const
	{
		auto component = static_component_type_of(type);

		if (component.is_scalar())
		{
			return Expression(component, value + s_swizzle[0]);
		}

		return static_zero(component);
	}

	Expression Expression::y() const
	{
		auto component = static_component_type_of(type);

		if (component.is_scalar() && type.vector_length() > 1)
		{
			return Expression(component, value + s_swizzle[1]);
		}

		return static_zero(component);
	}

	Expression Expression::z() const
	{
		auto component = static_component_type_of(type);

		if (component.is_scalar() && type.vector_length() > 2)
		{
			return Expression(component, value + s_swizzle[2]);
		}

		return static_zero(component);
	}

	Expression Expression::w() const
	{
		auto component = static_component_type_of(type);

		if (component.is_scalar() && type.vector_length() > 3)
		{
			return Expression(component, value + s_swizzle[3]);
		}

		return static_zero(component);
	}

	Expression Compiler::compile(InputPin* pin)
	{
		return {};
	}

	Expression Compiler::compile(OutputPin* pin)
	{
		return {};
	}

	OutputPin& OutputPin::link(InputPin* pin)
	{
		if (pin)
			pin->link(this);
		return *this;
	}

	OutputPin& OutputPin::unlink()
	{
		while (!m_links.empty())
		{
			InputPin* pin = *m_links.begin();
			pin->unlink();
		}
		return *this;
	}

	InputPin& InputPin::link(OutputPin* pin)
	{
		unlink();
		m_link = pin;
		pin->m_links.insert(this);
		return *this;
	}

	InputPin& InputPin::unlink()
	{
		if (m_link)
		{
			m_link->m_links.erase(this);
			m_link = nullptr;
		}
		return *this;
	}

	InputPin* Node::new_input(const String& name, ShaderParameterType type)
	{
		InputPin* pin = new InputPin();
		pin->m_name   = name;
		pin->m_node   = this;
		pin->m_index  = static_cast<uint16_t>(m_inputs.size());
		pin->m_type   = type;
		m_inputs.push_back(pin);
		return pin;
	}

	OutputPin* Node::new_output(const String& name, ShaderParameterType type)
	{
		OutputPin* pin = new OutputPin();
		pin->m_name    = name;
		pin->m_node    = this;
		pin->m_index   = static_cast<uint16_t>(m_inputs.size());
		pin->m_type    = type;
		m_outputs.push_back(pin);
		return pin;
	}

	Expression Node::script_compile(OutputPin* pin, Compiler& compiler)
	{
		Expression result;
		ScriptContext::execute(s_node_compile_output, &result, pin, &compiler);
		return result;
	}

	Expression Node::compile(OutputPin* pin, Compiler& compiler)
	{
		return Expression();
	}

	Node::~Node()
	{
		for (Pin* pin : m_inputs)
		{
			release(pin);
		}

		for (Pin* pin : m_outputs)
		{
			release(pin);
		}
	}

	MaterialRoot::MaterialRoot()
		: Node(), base_color(new_input("Base Color", ShaderParameterType::Float3)),//
		  opacity(new_input("Opacity", ShaderParameterType::Float3)),              //
		  emissive(new_input("Emissive", ShaderParameterType::Float)),             //
		  specular(new_input("Specular", ShaderParameterType::Float)),             //
		  metalness(new_input("Metalness", ShaderParameterType::Float)),           //
		  roughness(new_input("Roughness", ShaderParameterType::Float)),           //
		  ao(new_input("AO", ShaderParameterType::Float)),                         //
		  normal(new_input("Normal", ShaderParameterType::Float3)),                //
		  position_offset(new_input("Position Offset", ShaderParameterType::Float3))
	{}

	static void reflection_init()
	{
		using Reg = ScriptClassRegistrar;

		auto ref_class_info            = Reg::RefInfo();
		ref_class_info.implicit_handle = true;
		ref_class_info.no_count        = true;

		auto value_class_info                    = Reg::ValueInfo::from<Expression>();
		value_class_info.more_constructors       = true;
		value_class_info.has_constructor         = true;
		value_class_info.has_destructor          = true;
		value_class_info.has_assignment_operator = true;
		value_class_info.has_copy_constructor    = true;

		auto compiler   = Reg::reference_class("Engine::VisualMaterialGraph::Compiler", ref_class_info);
		auto input_pin  = Reg::reference_class("Engine::VisualMaterialGraph::InputPin", ref_class_info);
		auto output_pin = Reg::reference_class("Engine::VisualMaterialGraph::OutputPin", ref_class_info);
		auto expression = Reg::value_class("Engine::VisualMaterialGraph::Expression", sizeof(Expression), value_class_info);

		// Compiler class
		compiler.method("Expression compile(InputPin@ pin) final", method_of<Expression, InputPin*>(&Compiler::compile));
		compiler.method("Expression compile(OutputPin@ pin) final", method_of<Expression, OutputPin*>(&Compiler::compile));

		// Input pin class

		// Output pin class

		// Expression Class

		// clang-format off
		expression.behave(ScriptClassBehave::Construct, "void f()", Reg::constructor<Expression>);
		expression.behave(ScriptClassBehave::Construct, "void f(const Expression&)", Reg::constructor<Expression, const Expression&>);
		expression.behave(ScriptClassBehave::Construct, "void f(ShaderParameterType type, const string& expression)", Reg::constructor<Expression, ShaderParameterType, const String&>);
		expression.behave(ScriptClassBehave::Construct, "void f(bool value)", Reg::constructor<Expression, bool>);
		expression.behave(ScriptClassBehave::Construct, "void f(float value)", Reg::constructor<Expression, float>);
		expression.behave(ScriptClassBehave::Construct, "void f(double value)", Reg::constructor<Expression, double>);
		expression.behave(ScriptClassBehave::Construct, "void f(int32 value)", Reg::constructor<Expression, int32_t>);
		expression.behave(ScriptClassBehave::Construct, "void f(uint32 value)", Reg::constructor<Expression, uint32_t>);
		expression.behave(ScriptClassBehave::Destruct, "void f()", Reg::destructor<Expression>);
		// clang-format on

		expression.property("string value", &Expression::value);
		expression.property("ShaderParameterType type", &Expression::type);

		expression.method("Expression& opAssign(const Expression&)", Reg::assign<Expression, const Expression&>);
		expression.method("Expression x() const", &Expression::x);
		expression.method("Expression y() const", &Expression::y);
		expression.method("Expression z() const", &Expression::z);
		expression.method("Expression w() const", &Expression::w);
		expression.method("Expression& clear()", &Expression::clear);
		expression.method("bool is_valid() const", &Expression::is_valid);
		expression.method("Expression convert(ShaderParameterType type) const", &Expression::convert);

		// clang-format off
		expression.static_function("Expression static_zero(ShaderParameterType type)", &Expression::static_zero);
		expression.static_function("Expression static_half(ShaderParameterType type)", &Expression::static_half);
		expression.static_function("Expression static_one(ShaderParameterType type)", &Expression::static_one);
		expression.static_function("ShaderParameterType static_component_type_of(ShaderParameterType type)", &Expression::static_component_type_of);
		expression.static_function("ShaderParameterType static_resolve(ShaderParameterType type1, ShaderParameterType type2)", &Expression::static_resolve);
		expression.static_function("string static_typename_of(ShaderParameterType type)", &Expression::static_typename_of);
		// clang-format on

		// static ShaderParameterType component_type(ShaderParameterType type);
	}

	static ReflectionInitializeController init(reflection_init, "Engine::VisualMaterialGraph", {"Engine::ShaderParameterType"});
}// namespace Engine::VisualMaterialGraph
