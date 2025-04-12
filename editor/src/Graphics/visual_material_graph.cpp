#include <Core/engine_loading_controllers.hpp>
#include <Core/exception.hpp>
#include <Core/group.hpp>
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
				r.method("Expression compile(OutputPin@ pin, Compiler@ compiler)", trinex_scoped_void_method(This, compile));


		InputPin* (This::*method1)(const String&, ShaderParameterType)                       = &This::new_input;
		OutputPin* (This::*method2)(const String&, ShaderParameterType)                      = &This::new_output;
		InputPin* (This::*method3)(const String&, ShaderParameterType, ShaderParameterType)  = &This::new_input;
		OutputPin* (This::*method4)(const String&, ShaderParameterType, ShaderParameterType) = &This::new_output;

		// clang-format off
		r.method("InputPin@ new_input(const string& name, ShaderParameterType type) final", method1);
		r.method("OutputPin@ new_output(const string& name, ShaderParameterType type) final", method2);
		r.method("InputPin@ new_input(const string& name, ShaderParameterType type, ShaderParameterType default_type) final",method3);
		r.method("OutputPin@ new_output(const string& name, ShaderParameterType type, ShaderParameterType default_type) final", method4);
		// clang-format on

		ScriptEngine::on_terminate.push([]() { s_node_compile_output.release(); });
	}

	trinex_implement_class(Engine::VisualMaterialGraph::MaterialRoot, 0) {}

	template<typename T>
	struct DataTypeFormatter {
		static String format(const T& value, ShaderParameterType type, uint_t depth = 0)
		{
			if (depth == 0)
				return Strings::format("{}({})", Expression::static_typename_of(type), value);
			return Strings::format("{}", value);
		}
	};

	template<glm::length_t L, typename T, glm::qualifier Q>
	struct DataTypeFormatter<glm::vec<L, T, Q>> {
		static String format(const glm::vec<L, T, Q>& value, ShaderParameterType type, uint_t depth = 0)
		{
			String result = Expression::static_typename_of(type);
			result.push_back('(');

			type = type.make_scalar();
			result += DataTypeFormatter<T>::format(value.x, type, depth + 1);


			for (size_t i = 1; i < L; ++i)
			{
				result += ", ";
				result += DataTypeFormatter<T>::format(value[i], type, depth + 1);
			}

			result.push_back(')');
			return result;
		};
	};

	template<>
	struct DataTypeFormatter<Matrix3f> {
		static String format(const Matrix3f& value, ShaderParameterType type, uint_t depth = 0) { return {}; }
	};

	template<>
	struct DataTypeFormatter<Matrix4f> {
		static String format(const Matrix4f& value, ShaderParameterType type, uint_t depth = 0) { return {}; }
	};

	template<typename T, ShaderParameterType type_value>
	struct DefaultValueHolder : public Pin::DefaultValue {
		T value = T();

		byte* address() override { return reinterpret_cast<byte*>(&value); }
		ShaderParameterType type() const override { return type_value; };
		Expression compile() const override { return Expression(type_value, DataTypeFormatter<T>::format(value, type_value)); }
	};

	static Pin::DefaultValue* create_default_value_holder(ShaderParameterType type)
	{
		using T = ShaderParameterType;

		switch (type.value)
		{
			case T::Bool:
				return allocate<DefaultValueHolder<Vector1b, T::Bool>>();
			case T::Bool2:
				return allocate<DefaultValueHolder<Vector2b, T::Bool2>>();
			case T::Bool3:
				return allocate<DefaultValueHolder<Vector3b, T::Bool3>>();
			case T::Bool4:
				return allocate<DefaultValueHolder<Vector4b, T::Bool4>>();
			case T::Int:
				return allocate<DefaultValueHolder<Vector1i, T::Int>>();
			case T::Int2:
				return allocate<DefaultValueHolder<Vector2i, T::Int2>>();
			case T::Int3:
				return allocate<DefaultValueHolder<Vector3i, T::Int3>>();
			case T::Int4:
				return allocate<DefaultValueHolder<Vector4i, T::Int4>>();
			case T::UInt:
				return allocate<DefaultValueHolder<Vector1u, T::UInt>>();
			case T::UInt2:
				return allocate<DefaultValueHolder<Vector2u, T::UInt2>>();
			case T::UInt3:
				return allocate<DefaultValueHolder<Vector3u, T::UInt3>>();
			case T::UInt4:
				return allocate<DefaultValueHolder<Vector4u, T::UInt4>>();
			case T::Float:
				return allocate<DefaultValueHolder<Vector1f, T::Float>>();
			case T::Float2:
				return allocate<DefaultValueHolder<Vector2f, T::Float2>>();
			case T::Float3:
				return allocate<DefaultValueHolder<Vector3f, T::Float3>>();
			case T::Float4:
				return allocate<DefaultValueHolder<Vector4f, T::Float4>>();
			case T::Float3x3:
				return allocate<DefaultValueHolder<Matrix3f, T::Float3x3>>();
			case T::Float4x4:
				return allocate<DefaultValueHolder<Matrix4f, T::Float4x4>>();
			default:
				return nullptr;
		}
		return nullptr;
	}

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

	bool Expression::is_compatible_types(ShaderParameterType src, ShaderParameterType dst)
	{
		if (src == dst)
			return true;

		if ((src.is_scalar() || src.is_vector()) && (dst.is_scalar() || dst.is_vector()))
			return true;

		if (src.is_matrix() && dst.is_matrix())
			return true;
		return false;
	}

	ShaderParameterType Expression::static_make_float(ShaderParameterType self)
	{
		if (self.is_numeric())
		{
			byte len = self.vector_length();
			return ShaderParameterType(ShaderParameterType::Float).make_vector(len);
		}

		if (self.is_matrix())
			return self;

		return ShaderParameterType::Undefined;
	}

	ShaderParameterType Expression::static_vector_clamp(ShaderParameterType self, byte min, byte max)
	{
		if (self.is_numeric())
		{
			byte len        = self.vector_length();
			byte normalized = glm::clamp<byte>(glm::clamp(len, min, max), 1, 4);

			if (normalized != len)
				return self.make_vector(normalized);
			return self;
		}

		return ShaderParameterType::Undefined;
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

	Expression Expression::convert(ShaderParameterType dst) const
	{
		if (type == dst || value.empty())
			return *this;

		if ((dst.is_vector() || dst.is_scalar()) && (type.is_vector() || type.is_scalar()))
		{
			size_t src_components = type.vector_length();
			size_t dst_components = dst.vector_length();

			const auto src_component_type = static_component_type_of(type);
			const auto dst_component_type = static_component_type_of(dst);

			String result;

			const bool need_wrap = !(src_component_type == dst_component_type && src_components > dst_components);

			if (need_wrap)
				result = Strings::format("{}({}", static_typename_of(dst), value);
			else
				result = value;


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

			return Expression(dst, result);
		}


		throw EngineException("Unsupported expression type for cast");
	}

	Expression Expression::vector_length() const
	{
		if (type.is_vector())
		{
			Expression result = to_floating();
			result.type       = ShaderParameterType::Float;
			result.value      = Strings::format("length({})", result.value);
			return result;
		}

		if (type.is_scalar())
			return to_floating();
		return Expression();
	}

	Compiler& Compiler::add_include(const StringView& include)
	{
		m_includes.insert(String(include));
		return *this;
	}

	Expression Compiler::make_variable(ShaderParameterType type)
	{
		Identifier id   = m_stage_locals[m_stage].size() + m_globals.size();
		String var_name = Strings::format("trx_var_{}", id);

		String var = Strings::format("{} {}", Expression::static_typename_of(type), var_name);
		m_stage_locals[m_stage].push_back(var);
		return Expression(type, var_name);
	}

	Expression Compiler::make_variable(const Expression& expression)
	{
		if (expression.value.starts_with("trx_var_"))
			return expression;

		Identifier id = m_stage_locals[m_stage].size() + m_globals.size();

		String type     = Expression::static_typename_of(expression.type);
		String var_name = Strings::format("trx_var_{}", id);
		String var      = Strings::format("{} {} = {}", type, var_name, expression.value);
		m_stage_locals[m_stage].push_back(var);

		return Expression(expression.type, var_name);
	}

	Expression Compiler::compile_default(Pin* pin)
	{
		if (auto default_value = pin->default_value())
		{
			return default_value->compile();
		}

		return {};
	}

	Expression Compiler::compile(InputPin* pin)
	{
		OutputPin* output_pin = pin->linked_pin();

		if (output_pin)
		{
			Expression expression = compile(output_pin);
			if (!pin->type().is_meta())
				expression = expression.convert(pin->type());
			return expression;
		}

		if (auto default_value = pin->default_value())
		{
			return default_value->compile();
		}

		return {};
	}

	Expression Compiler::compile(OutputPin* pin)
	{
		auto it = m_expression.find(pin);

		if (it != m_expression.end())
			return it->second;

		Expression expression = pin->node()->compile(pin, *this);

		if (pin->links_count() > 1)
			expression = make_variable(expression);

		m_expression[pin] = expression;
		return expression;
	}

	template<typename T>
	static String compile_expressions(const T& expression, size_t tabs)
	{
		const String spacing(tabs, '\t');

		String source;
		size_t required_size = 0;

		for (const String& expression : expression) required_size += spacing.size() + expression.size() + 1;
		source.reserve(required_size);

		for (const String& expression : expression)
		{
			source += spacing;
			source += expression;
			source += '\n';
		}

		return source;
	}

	String Compiler::compile_includes(size_t tabs) const
	{
		return compile_expressions(m_includes, tabs);
	}

	String Compiler::compile_global_expressions(size_t tabs) const
	{
		return compile_expressions(m_globals, tabs);
	}

	String Compiler::compile_local_expressions(size_t tabs) const
	{
		return compile_expressions(m_stage_locals[m_stage], tabs);
	}

	Pin::~Pin()
	{
		if (m_default_value)
		{
			release(m_default_value);
		}
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

	InputPin* Node::new_input(const String& name, ShaderParameterType type, ShaderParameterType default_value_type)
	{
		auto pin                                = new_input(name, type);
		static_cast<Pin*>(pin)->m_default_value = create_default_value_holder(default_value_type);
		return pin;
	}

	OutputPin* Node::new_output(const String& name, ShaderParameterType type, ShaderParameterType default_value_type)
	{
		auto pin                                = new_output(name, type);
		static_cast<Pin*>(pin)->m_default_value = create_default_value_holder(default_value_type);
		return pin;
	}

	Expression Node::script_compile(OutputPin* pin, Compiler& compiler)
	{
		Expression result;
		ScriptContext::execute(this, s_node_compile_output, &result, pin, &compiler);
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
		: Node(), base_color(new_input("Base Color", ShaderParameterType::Float3, ShaderParameterType::Float3)),//
		  opacity(new_input("Opacity", ShaderParameterType::Float, ShaderParameterType::Float)),                //
		  emissive(new_input("Emissive Color", ShaderParameterType::Float3, ShaderParameterType::Float3)),      //
		  specular(new_input("Specular", ShaderParameterType::Float, ShaderParameterType::Float)),              //
		  metalness(new_input("Metalness", ShaderParameterType::Float, ShaderParameterType::Float)),            //
		  roughness(new_input("Roughness", ShaderParameterType::Float, ShaderParameterType::Float)),            //
		  ao(new_input("AO", ShaderParameterType::Float, ShaderParameterType::Float)),                          //
		  normal(new_input("Normal", ShaderParameterType::Float3, ShaderParameterType::Float3)),                //
		  position_offset(new_input("Position Offset", ShaderParameterType::Float3, ShaderParameterType::Float3))
	{
		opacity->default_value()->ref<float>()   = 1.0f;
		specular->default_value()->ref<float>()  = 0.5f;
		normal->default_value()->ref<Vector3f>() = {0.5f, 0.5f, 1.0f};
		ao->default_value()->ref<float>()        = 1.f;
	}

	static void script_class_node_group(Refl::Class* self, const String& group_name)
	{
		if (!self->is_a<Node>())
		{
			throw EngineException(
					"Cannot use 'node_group' with classes, which is not derived from Engine::VisualMaterialGraph::Node!");
		}

		String full_group_name = Strings::format("Engine::VisualMaterialGraph::Nodes::{}", group_name);
		Group::find(full_group_name, true)->add_struct(self);
	}

	static void register_metadata_functions()
	{
		ScriptClassRegistrar r = ScriptClassRegistrar::existing_class("Engine::Refl::Class");
		r.method("void node_group(const string& group_name) const final", script_class_node_group);
	}

	template<typename T>
	static void register_pin_methods(ScriptClassRegistrar& r)
	{
		r.method("const string& name() const", &Pin::name);
		r.method("Node@ node() const", &Pin::node);
		r.method("ShaderParameterType type() const", &Pin::type);
		r.method("uint16 index() const", &Pin::index);
		r.method("uint64 id() const", &Pin::id);
		r.method("uint64 links_count() const", &T::links_count);
	}

	static void reflection_init()
	{
		register_metadata_functions();

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

		register_pin_methods<InputPin>(input_pin);
		register_pin_methods<OutputPin>(output_pin);

		// Compiler class
		compiler.method("Expression make_variable(ShaderParameterType type) final",
						method_of<Expression, ShaderParameterType>(&Compiler::make_variable));
		compiler.method("Expression make_variable(const Expression& expression) final",
						method_of<Expression, const Expression&>(&Compiler::make_variable));

		compiler.method("Compiler& add_include(const StringView& file) final", &Compiler::add_include);
		compiler.method("Expression compile_default(InputPin@ pin) final", &Compiler::compile_default);
		compiler.method("Expression compile_default(OutputPin@ pin) final", &Compiler::compile_default);
		compiler.method("Expression compile(InputPin@ pin) final", method_of<Expression, InputPin*>(&Compiler::compile));
		compiler.method("Expression compile(OutputPin@ pin) final", method_of<Expression, OutputPin*>(&Compiler::compile));

		// Input pin class

		// Output pin class

		// Expression Class

		// clang-format off
		expression.behave(ScriptClassBehave::Construct, "void f()", Reg::constructor<Expression>);
		expression.behave(ScriptClassBehave::Construct, "void f(const Expression&)", Reg::constructor<Expression, const Expression&>);
		expression.behave(ScriptClassBehave::Construct, "void f(ShaderParameterType type, const string& expression)", Reg::constructor<Expression, ShaderParameterType, const String&>);
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
		expression.method("Expression to_floating() const", &Expression::to_floating);
		expression.method("Expression vector_length() const", &Expression::vector_length);

		// clang-format off
		expression.static_function("Expression static_zero(ShaderParameterType type)", &Expression::static_zero);
		expression.static_function("Expression static_half(ShaderParameterType type)", &Expression::static_half);
		expression.static_function("Expression static_one(ShaderParameterType type)", &Expression::static_one);
		expression.static_function("ShaderParameterType static_component_type_of(ShaderParameterType type)", &Expression::static_component_type_of);
		expression.static_function("ShaderParameterType static_resolve(ShaderParameterType type1, ShaderParameterType type2)", &Expression::static_resolve);
		expression.static_function("string static_typename_of(ShaderParameterType type)", &Expression::static_typename_of);

		expression.static_function("ShaderParameterType static_make_float(ShaderParameterType self)", &Expression::static_make_float);
		expression.static_function("ShaderParameterType static_vector_clamp(ShaderParameterType self, uint8 min, uint8 max)", &Expression::static_vector_clamp);
		expression.static_function("ShaderParameterType static_make_vector(ShaderParameterType self, uint8 len)", &Expression::static_make_vector);
		expression.static_function("ShaderParameterType static_make_scalar(ShaderParameterType self)", &Expression::static_make_scalar);
		expression.static_function("bool static_is_scalar(ShaderParameterType self)", &Expression::static_is_scalar);
		expression.static_function("bool static_is_vector(ShaderParameterType self)", &Expression::static_is_vector);
		expression.static_function("bool static_is_matrix(ShaderParameterType self)", &Expression::static_is_matrix);
		expression.static_function("bool static_is_numeric(ShaderParameterType self)", &Expression::static_is_numeric);
		expression.static_function("bool static_is_meta(ShaderParameterType self)", &Expression::static_is_meta);
		expression.static_function("uint16 static_type_index(ShaderParameterType self)", &Expression::static_type_index);
		expression.static_function("uint8 static_vector_length(ShaderParameterType self)", &Expression::static_vector_length);

		// clang-format on

		// static ShaderParameterType component_type(ShaderParameterType type);
	}

	static ReflectionInitializeController init(reflection_init, "Engine::VisualMaterialGraph",
											   {"Engine::ShaderParameterType", "Engine::Refl::Class",
												"Engine::VisualMaterialGraph::Node"});
}// namespace Engine::VisualMaterialGraph
