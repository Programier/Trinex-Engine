#include <Core/engine_loading_controllers.hpp>
#include <Core/exception.hpp>
#include <Core/group.hpp>
#include <Core/logger.hpp>
#include <Core/memory.hpp>
#include <Core/reflection/class.hpp>
#include <Graphics/visual_material.hpp>
#include <Graphics/visual_material_graph.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <utility>

namespace Engine::VisualMaterialGraph
{
	static ScriptFunction s_node_compile_output;
	static ScriptFunction s_node_render;

	trinex_implement_class(Engine::VisualMaterialGraph::Node, Refl::Class::IsScriptable)
	{
		auto r = ScriptClassRegistrar::reference_class(static_class_instance());

		s_node_compile_output =
		        r.method("Expression compile(OutputPin@ pin, Compiler@ compiler)", trinex_scoped_void_method(This, compile));

		s_node_render = r.method("void render()", trinex_scoped_void_method(This, render));

		InputPin* (This::*method1)(const String&, RHIShaderParameterType)                          = &This::new_input;
		OutputPin* (This::*method2)(const String&, RHIShaderParameterType)                         = &This::new_output;
		InputPin* (This::*method3)(const String&, RHIShaderParameterType, RHIShaderParameterType)  = &This::new_input;
		OutputPin* (This::*method4)(const String&, RHIShaderParameterType, RHIShaderParameterType) = &This::new_output;

		// clang-format off
		r.method("InputPin@ new_input(const string& name, RHIShaderParameterType type) final", method1);
		r.method("OutputPin@ new_output(const string& name, RHIShaderParameterType type) final", method2);
		r.method("InputPin@ new_input(const string& name, RHIShaderParameterType type, RHIShaderParameterType default_type) final",method3);
		r.method("OutputPin@ new_output(const string& name, RHIShaderParameterType type, RHIShaderParameterType default_type) final", method4);
		r.method("const Vector<InputPin@>& inputs() const final", &This::inputs);
		r.method("const Vector<OutputPin@>& outputs() const final", &This::outputs);
		// clang-format on

		ScriptEngine::on_terminate.push([]() {
			s_node_compile_output.release();
			s_node_render.release();
		});
	}

	template<typename T>
	struct DataTypeFormatter {
		static String format(const T& value, RHIShaderParameterType type, uint_t depth = 0)
		{
			if (depth == 0)
				return Strings::format("{}({})", Expression::static_typename_of(type), value);
			return Strings::format("{}", value);
		}
	};

	template<glm::length_t L, typename T, glm::qualifier Q>
	struct DataTypeFormatter<glm::vec<L, T, Q>> {
		static String format(const glm::vec<L, T, Q>& value, RHIShaderParameterType type, uint_t depth = 0)
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
		static String format(const Matrix3f& value, RHIShaderParameterType type, uint_t depth = 0) { return {}; }
	};

	template<>
	struct DataTypeFormatter<Matrix4f> {
		static String format(const Matrix4f& value, RHIShaderParameterType type, uint_t depth = 0) { return {}; }
	};

	template<typename T, RHIShaderParameterType type_value>
	struct DefaultValueHolder : public Pin::DefaultValue {
		T value = T();

		byte* address() override { return reinterpret_cast<byte*>(&value); }
		RHIShaderParameterType type() const override { return type_value; };
		Expression compile() const override { return Expression(type_value, DataTypeFormatter<T>::format(value, type_value)); }
	};

	static Pin::DefaultValue* create_default_value_holder(RHIShaderParameterType type)
	{
		using T = RHIShaderParameterType;

		switch (type.value)
		{
			case T::Bool: return allocate<DefaultValueHolder<Vector1b, T::Bool>>();
			case T::Bool2: return allocate<DefaultValueHolder<Vector2b, T::Bool2>>();
			case T::Bool3: return allocate<DefaultValueHolder<Vector3b, T::Bool3>>();
			case T::Bool4: return allocate<DefaultValueHolder<Vector4b, T::Bool4>>();
			case T::Int: return allocate<DefaultValueHolder<Vector1i, T::Int>>();
			case T::Int2: return allocate<DefaultValueHolder<Vector2i, T::Int2>>();
			case T::Int3: return allocate<DefaultValueHolder<Vector3i, T::Int3>>();
			case T::Int4: return allocate<DefaultValueHolder<Vector4i, T::Int4>>();
			case T::UInt: return allocate<DefaultValueHolder<Vector1u, T::UInt>>();
			case T::UInt2: return allocate<DefaultValueHolder<Vector2u, T::UInt2>>();
			case T::UInt3: return allocate<DefaultValueHolder<Vector3u, T::UInt3>>();
			case T::UInt4: return allocate<DefaultValueHolder<Vector4u, T::UInt4>>();
			case T::Float: return allocate<DefaultValueHolder<Vector1f, T::Float>>();
			case T::Float2: return allocate<DefaultValueHolder<Vector2f, T::Float2>>();
			case T::Float3: return allocate<DefaultValueHolder<Vector3f, T::Float3>>();
			case T::Float4: return allocate<DefaultValueHolder<Vector4f, T::Float4>>();
			case T::Float3x3: return allocate<DefaultValueHolder<Matrix3f, T::Float3x3>>();
			case T::Float4x4: return allocate<DefaultValueHolder<Matrix4f, T::Float4x4>>();
			default: return nullptr;
		}
		return nullptr;
	}

	const char* Expression::s_swizzle[4] = {".x", ".y", ".z", ".w"};

	Expression Expression::static_zero(RHIShaderParameterType type)
	{
		switch (type)
		{
			case RHIShaderParameterType::Bool: return Expression(type, "false");
			case RHIShaderParameterType::Bool2: return Expression(type, "bool2(false, false)");
			case RHIShaderParameterType::Bool3: return Expression(type, "bool3(false, false, false)");
			case RHIShaderParameterType::Bool4: return Expression(type, "bool4(false, false, false)");

			case RHIShaderParameterType::Int: return Expression(type, "int(0)");
			case RHIShaderParameterType::Int2: return Expression(type, "int2(0, 0)");
			case RHIShaderParameterType::Int3: return Expression(type, "int3(0, 0, 0)");
			case RHIShaderParameterType::Int4: return Expression(type, "int4(0, 0, 0, 0)");

			case RHIShaderParameterType::UInt: return Expression(type, "uint(0)");
			case RHIShaderParameterType::UInt2: return Expression(type, "uint2(0, 0)");
			case RHIShaderParameterType::UInt3: return Expression(type, "uint3(0, 0, 0)");
			case RHIShaderParameterType::UInt4: return Expression(type, "uint4(0, 0, 0, 0)");

			case RHIShaderParameterType::Float: return Expression(type, "float(0.f)");
			case RHIShaderParameterType::Float2: return Expression(type, "float2(0.f, 0.f)");
			case RHIShaderParameterType::Float3: return Expression(type, "float3(0.f, 0.f, 0.f)");
			case RHIShaderParameterType::Float4: return Expression(type, "float4(0.f, 0.f, 0.f, 0.f)");
			case RHIShaderParameterType::Float3x3: return Expression(type, "float3x3(0.f)");
			case RHIShaderParameterType::Float4x4: return Expression(type, "float4x4(0.f)");
			default: throw EngineException("Unsupported shader parameter type!");
		}
	}

	Expression Expression::static_half(RHIShaderParameterType type)
	{
		switch (type)
		{
			case RHIShaderParameterType::Bool: return Expression(type, "false");
			case RHIShaderParameterType::Bool2: return Expression(type, "bool2(false, false)");
			case RHIShaderParameterType::Bool3: return Expression(type, "bool3(false, false, false)");
			case RHIShaderParameterType::Bool4: return Expression(type, "bool4(false, false, false)");

			case RHIShaderParameterType::Int: return Expression(type, "int(0)");
			case RHIShaderParameterType::Int2: return Expression(type, "int2(0, 0)");
			case RHIShaderParameterType::Int3: return Expression(type, "int3(0, 0, 0)");
			case RHIShaderParameterType::Int4: return Expression(type, "int4(0, 0, 0, 0)");

			case RHIShaderParameterType::UInt: return Expression(type, "uint(0)");
			case RHIShaderParameterType::UInt2: return Expression(type, "uint2(0, 0)");
			case RHIShaderParameterType::UInt3: return Expression(type, "uint3(0, 0, 0)");
			case RHIShaderParameterType::UInt4: return Expression(type, "uint4(0, 0, 0, 0)");

			case RHIShaderParameterType::Float: return Expression(type, "float(0.5f)");
			case RHIShaderParameterType::Float2: return Expression(type, "float2(0.5f, 0.5f)");
			case RHIShaderParameterType::Float3: return Expression(type, "float3(0.5f, 0.5f, 0.5f)");
			case RHIShaderParameterType::Float4: return Expression(type, "float4(0.5f, 0.5f, 0.5f, 0.5f)");
			case RHIShaderParameterType::Float3x3: return Expression(type, "float3x3(0.5f)");
			case RHIShaderParameterType::Float4x4: return Expression(type, "float4x4(0.5f)");
			default: throw EngineException("Unsupported shader parameter type!");
		}
	}

	Expression Expression::static_one(RHIShaderParameterType type)
	{
		switch (type)
		{
			case RHIShaderParameterType::Bool: return Expression(type, "true");
			case RHIShaderParameterType::Bool2: return Expression(type, "bool2(true, true)");
			case RHIShaderParameterType::Bool3: return Expression(type, "bool3(true, true, true)");
			case RHIShaderParameterType::Bool4: return Expression(type, "bool4(true, true, true)");

			case RHIShaderParameterType::Int: return Expression(type, "int(1)");
			case RHIShaderParameterType::Int2: return Expression(type, "int2(1, 1)");
			case RHIShaderParameterType::Int3: return Expression(type, "int3(1, 1, 1)");
			case RHIShaderParameterType::Int4: return Expression(type, "int4(1, 1, 1, 1)");

			case RHIShaderParameterType::UInt: return Expression(type, "uint(1)");
			case RHIShaderParameterType::UInt2: return Expression(type, "uint2(1, 1)");
			case RHIShaderParameterType::UInt3: return Expression(type, "uint3(1, 1, 1)");
			case RHIShaderParameterType::UInt4: return Expression(type, "uint4(1, 1, 1, 1)");

			case RHIShaderParameterType::Float: return Expression(type, "float(1.f)");
			case RHIShaderParameterType::Float2: return Expression(type, "float2(1.f, 1.f)");
			case RHIShaderParameterType::Float3: return Expression(type, "float3(1.f, 1.f, 1.f)");
			case RHIShaderParameterType::Float4: return Expression(type, "float4(1.f, 1.f, 1.f, 1.f)");
			case RHIShaderParameterType::Float3x3: return Expression(type, "float3x3(1.f)");
			case RHIShaderParameterType::Float4x4: return Expression(type, "float4x4(1.f)");
			default: throw EngineException("Unsupported shader parameter type!");
		}
	}

	RHIShaderParameterType Expression::static_component_type_of(RHIShaderParameterType type)
	{
		switch (type)
		{
			case RHIShaderParameterType::Bool2:
			case RHIShaderParameterType::Bool3:
			case RHIShaderParameterType::Bool4: return RHIShaderParameterType::Bool;

			case RHIShaderParameterType::Int2:
			case RHIShaderParameterType::Int3:
			case RHIShaderParameterType::Int4: return RHIShaderParameterType::Int;

			case RHIShaderParameterType::UInt2:
			case RHIShaderParameterType::UInt3:
			case RHIShaderParameterType::UInt4: return RHIShaderParameterType::UInt;

			case RHIShaderParameterType::Float2:
			case RHIShaderParameterType::Float3:
			case RHIShaderParameterType::Float4:
			case RHIShaderParameterType::Float3x3:
			case RHIShaderParameterType::Float4x4: return RHIShaderParameterType::Float;

			default: return type;
		}
	}

	RHIShaderParameterType Expression::static_resolve(RHIShaderParameterType type1, RHIShaderParameterType type2)
	{
		if (type1 == type2)
			return type1;

		if ((type1.is_scalar() || type1.is_vector()) && (type2.is_scalar() || type2.is_vector()))
		{
			auto type_component1 = static_component_type_of(type1);
			auto type_component2 = static_component_type_of(type2);
			byte result_len      = glm::max(type1.vector_length(), type2.vector_length());

			if (type_component1 == RHIShaderParameterType::Float || type_component2 == RHIShaderParameterType::Float)
				return RHIShaderParameterType(RHIShaderParameterType::Float).make_vector(result_len);

			if (type_component1 == RHIShaderParameterType::Bool || type_component2 == RHIShaderParameterType::Bool)
				return RHIShaderParameterType(RHIShaderParameterType::Bool).make_vector(result_len);

			if (type_component1 == RHIShaderParameterType::UInt || type_component2 == RHIShaderParameterType::UInt)
				return RHIShaderParameterType(RHIShaderParameterType::UInt).make_vector(result_len);

			return RHIShaderParameterType(RHIShaderParameterType::Int).make_vector(result_len);
		}

		return RHIShaderParameterType();
	}

	RHIShaderParameterType Expression::static_resolve(RHIShaderParameterType type1, RHIShaderParameterType type2,
	                                                  RHIShaderParameterType type3)
	{
		return static_resolve(static_resolve(type1, type2), type3);
	}

	RHIShaderParameterType Expression::static_resolve(RHIShaderParameterType type1, RHIShaderParameterType type2,
	                                                  RHIShaderParameterType type3, RHIShaderParameterType type4)
	{
		return static_resolve(static_resolve(type1, type2, type3), type4);
	}

	String Expression::static_typename_of(RHIShaderParameterType type)
	{
		// clang-format off
		switch (type)
		{
			case RHIShaderParameterType::Bool: return "bool";
			case RHIShaderParameterType::Bool2: return "bool2";
			case RHIShaderParameterType::Bool3: return "bool3";
			case RHIShaderParameterType::Bool4: return "bool4";
			case RHIShaderParameterType::Int: return "int";
			case RHIShaderParameterType::Int2: return "int2";
			case RHIShaderParameterType::Int3: return "int3";
			case RHIShaderParameterType::Int4: return "int4";
			case RHIShaderParameterType::UInt: return "uint";
			case RHIShaderParameterType::UInt2: return "uint2";
			case RHIShaderParameterType::UInt3: return "uint3";
			case RHIShaderParameterType::UInt4: return "uint4";
			case RHIShaderParameterType::Float: return "float";
			case RHIShaderParameterType::Float2: return "float2";
			case RHIShaderParameterType::Float3: return "float3";
			case RHIShaderParameterType::Float4: return "float4";
			case RHIShaderParameterType::Float3x3: return "float3x3";
			case RHIShaderParameterType::Float4x4: return "float4x4";
			case RHIShaderParameterType::Sampler: return "SamplerState";
			case RHIShaderParameterType::Sampler2D: return "Sampler2D";
			case RHIShaderParameterType::Texture2D: return "Texture2D";
			default:
				break;
		}
		// clang-format on
		throw EngineException("Unsupported type!");
	}

	bool Expression::is_compatible_types(RHIShaderParameterType src, RHIShaderParameterType dst)
	{
		if (src == dst)
			return true;

		if ((src.is_scalar() || src.is_vector()) && (dst.is_scalar() || dst.is_vector()))
			return true;

		if (src.is_matrix() && dst.is_matrix())
			return true;

		// We can do upcast, but cannot do downcast, so, source type must have all bits of destination type to allow casting
		return (src & dst) == dst;
	}

	RHIShaderParameterType Expression::static_make_float(RHIShaderParameterType self)
	{
		if (self.is_numeric())
		{
			byte len = self.vector_length();
			return RHIShaderParameterType(RHIShaderParameterType::Float).make_vector(len);
		}

		if (self.is_matrix())
			return self;

		return RHIShaderParameterType::Undefined;
	}

	RHIShaderParameterType Expression::static_vector_clamp(RHIShaderParameterType self, byte min, byte max)
	{
		if (self.is_numeric())
		{
			byte len        = self.vector_length();
			byte normalized = glm::clamp<byte>(glm::clamp(len, min, max), 1, 4);

			if (normalized != len)
				return self.make_vector(normalized);
			return self;
		}

		return RHIShaderParameterType::Undefined;
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

	Expression Expression::convert(RHIShaderParameterType dst) const
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
			result.type       = RHIShaderParameterType::Float;
			result.value      = Strings::format("length({})", result.value);
			return result;
		}

		if (type.is_scalar())
			return to_floating();
		return Expression();
	}

	Compiler::Compiler() = default;

	Compiler::~Compiler()
	{
		for (Node* node : m_temp_nodes)
		{
			node->remove_reference();
		}
	}

	Node* Compiler::create_temp_node(Refl::Class* node_class, uint16_t id)
	{
		if (!node_class->is_a<Node>())
			return nullptr;

		Node* node = Object::instance_cast<Node>(node_class->create_object());

		if (node)
		{
			node->add_reference();
			node->change_id(id);
			m_temp_nodes.insert(node);
		}

		return node;
	}

	String Compiler::next_var_name() const
	{
		Identifier id = m_stage_locals[m_stage].size() + m_globals.size();
		return Strings::format("trx_var_{}", id);
	}

	String Compiler::static_uniform_parameter_name(Node* node)
	{
		return static_uniform_parameter_name(node->class_instance(), node->id());
	}

	String Compiler::static_uniform_parameter_name(Refl::Class* node_class, uint16_t id)
	{
		String node_name = Strings::to_lower(node_class->name());
		return Strings::format("trx_var_{}_{}", node_name, id);
	}

	Compiler& Compiler::add_redirection(Node* node, Identifier id)
	{
		RedirectionKey key;
		key.node_class = node->class_instance();
		key.id         = id;

		m_redirections[key] = node;
		return *this;
	}

	Node* Compiler::find_redirection(Refl::Class* node_class, Identifier id)
	{
		RedirectionKey key;
		key.node_class = node_class;
		key.id         = id;

		auto it = m_redirections.find(key);

		if (it != m_redirections.end())
			return it->second;

		return nullptr;
	}

	Compiler& Compiler::add_include(const StringView& include)
	{
		m_includes.insert(String(include));
		return *this;
	}

	Expression Compiler::make_uniform(RHIShaderParameterType type, const String& name_override)
	{
		if (name_override.empty())
		{
			String var_name   = static_uniform_parameter_name(m_current_node);
			String expression = Strings::format("uniform {} {}", Expression::static_typename_of(type), var_name);
			m_globals.insert(expression);
			m_var_names.insert(var_name);
			return Expression(type, var_name);
		}
		else
		{
			if (m_param_names.contains(name_override))
				return Expression();

			m_param_names.insert(name_override);

			String var_name   = static_uniform_parameter_name(m_current_node);
			String expression = Strings::format("[name(\"{}\")] uniform {} {}", name_override,
			                                    Expression::static_typename_of(type), var_name);
			m_globals.insert(expression);
			m_var_names.insert(var_name);
			return Expression(type, var_name);
		}
	}

	Expression Compiler::make_variable(RHIShaderParameterType type)
	{
		String var_name = next_var_name();

		String var = Strings::format("{} {}", Expression::static_typename_of(type), var_name);
		m_stage_locals[m_stage].push_back(var);
		m_var_names.insert(var_name);
		return Expression(type, var_name);
	}

	Expression Compiler::make_variable(const Expression& expression)
	{
		if (m_var_names.contains(expression.value))
			return expression;

		String type     = Expression::static_typename_of(expression.type);
		String var_name = next_var_name();
		String var      = Strings::format("{} {} = {}", type, var_name, expression.value);
		m_stage_locals[m_stage].push_back(var);

		m_var_names.insert(var_name);
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

		return compile_default(pin);
	}

	Expression Compiler::compile(OutputPin* pin)
	{
		auto it = m_expression.find(pin);

		if (it != m_expression.end())
			return it->second;

		Node* node = pin->node();

		std::swap(node, m_current_node);
		Expression expression = m_current_node->compile(pin, *this);
		std::swap(node, m_current_node);

		if (pin->links_count() > 1)
			expression = make_variable(expression);

		m_expression[pin] = expression;
		return expression;
	}

	template<typename T>
	static String compile_expressions(const T& expression, size_t tabs, StringView ending = ";\n")
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
			source += ending;
		}

		return source;
	}

	String Compiler::compile_includes(size_t tabs) const
	{
		return compile_expressions(m_includes, tabs, "\n");
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

	void Node::static_node_group(Refl::Class* node_class, const String& group)
	{
		if (!node_class->is_a<Node>())
		{
			throw EngineException(
			        "Cannot use 'node_group' with classes, which is not derived from Engine::VisualMaterialGraph::Node!");
		}

		String full_group_name = Strings::format("Engine::VisualMaterialGraph::Nodes::{}", group);
		Group::find(full_group_name, true)->add_struct(node_class);
	}

	InputPin* Node::new_input(const String& name, RHIShaderParameterType type)
	{
		InputPin* pin = new InputPin();
		pin->m_name   = name;
		pin->m_node   = this;
		pin->m_index  = static_cast<uint16_t>(m_inputs.size());
		pin->m_type   = type;
		m_inputs.push_back(pin);
		return pin;
	}

	OutputPin* Node::new_output(const String& name, RHIShaderParameterType type)
	{
		OutputPin* pin = new OutputPin();
		pin->m_name    = name;
		pin->m_node    = this;
		pin->m_index   = static_cast<uint16_t>(m_outputs.size());
		pin->m_type    = type;
		m_outputs.push_back(pin);
		return pin;
	}

	InputPin* Node::new_input(const String& name, RHIShaderParameterType type, RHIShaderParameterType default_value_type)
	{
		auto pin                                = new_input(name, type);
		static_cast<Pin*>(pin)->m_default_value = create_default_value_holder(default_value_type);
		return pin;
	}

	OutputPin* Node::new_output(const String& name, RHIShaderParameterType type, RHIShaderParameterType default_value_type)
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

	void Node::script_render()
	{
		ScriptContext::execute(this, s_node_render);
	}

	Node& Node::on_property_changed(const Refl::PropertyChangedEvent& event)
	{
		Super::on_property_changed(event);

		if (auto material = instance_cast<VisualMaterial>(owner()))
		{
			post_compile(material);
		}

		return *this;
	}

	Expression Node::compile(OutputPin* pin, Compiler& compiler)
	{
		return Expression();
	}

	Node& Node::render()
	{
		return *this;
	}

	Node& Node::change_id(uint16_t id)
	{
		m_id = id;
		return *this;
	}

	Node& Node::post_compile(VisualMaterial* material)
	{
		return *this;
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

	static void register_metadata_functions()
	{
		ScriptClassRegistrar r = ScriptClassRegistrar::existing_class("Engine::Refl::Class");
		r.method("void node_group(const string& group_name) const final", Node::static_node_group);
	}

	template<typename T>
	static void register_pin_methods(ScriptClassRegistrar& r)
	{
		r.method("const string& name() const", &Pin::name);
		r.method("Node@ node() const", &Pin::node);
		r.method("RHIShaderParameterType type() const", &Pin::type);
		r.method("uint16 index() const", &Pin::index);
		r.method("uint64 links_count() const", &T::links_count);
	}

	static void reflection_init()
	{
		using SPType = RHIShaderParameterType;
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
		compiler.method("Expression make_uniform(RHIShaderParameterType type, const string& name_override = \"\") final",
		                &Compiler::make_uniform);
		compiler.method("Expression make_variable(RHIShaderParameterType type) final",
		                method_of<Expression, RHIShaderParameterType>(&Compiler::make_variable));
		compiler.method("Expression make_variable(const Expression& expression) final",
		                method_of<Expression, const Expression&>(&Compiler::make_variable));

		compiler.method("Compiler& add_include(const StringView& file) final", &Compiler::add_include);
		compiler.method("Expression compile_default(InputPin@ pin) final", &Compiler::compile_default);
		compiler.method("Expression compile_default(OutputPin@ pin) final", &Compiler::compile_default);
		compiler.method("Expression compile(InputPin@ pin) final", method_of<Expression, InputPin*>(&Compiler::compile));
		compiler.method("Expression compile(OutputPin@ pin) final", method_of<Expression, OutputPin*>(&Compiler::compile));
		compiler.method("bool is_vertex_stage() const", &Compiler::is_vertex_stage);
		compiler.method("bool is_fragment_stage() const", &Compiler::is_fragment_stage);

		// Input pin class

		// Output pin class

		// Expression Class

		// clang-format off
		expression.behave(ScriptClassBehave::Construct, "void f()", Reg::constructor<Expression>);
		expression.behave(ScriptClassBehave::Construct, "void f(const Expression&)", Reg::constructor<Expression, const Expression&>);
		expression.behave(ScriptClassBehave::Construct, "void f(RHIShaderParameterType type, const string& expression)", Reg::constructor<Expression, RHIShaderParameterType, const String&>);
		expression.behave(ScriptClassBehave::Destruct, "void f()", Reg::destructor<Expression>);
		// clang-format on

		expression.property("string value", &Expression::value);
		expression.property("RHIShaderParameterType type", &Expression::type);

		expression.method("Expression& opAssign(const Expression&)", Reg::assign<Expression, const Expression&>);
		expression.method("Expression x() const", &Expression::x);
		expression.method("Expression y() const", &Expression::y);
		expression.method("Expression z() const", &Expression::z);
		expression.method("Expression w() const", &Expression::w);
		expression.method("Expression& clear()", &Expression::clear);
		expression.method("bool is_valid() const", &Expression::is_valid);
		expression.method("Expression convert(RHIShaderParameterType type) const", &Expression::convert);
		expression.method("Expression to_floating() const", &Expression::to_floating);
		expression.method("Expression vector_length() const", &Expression::vector_length);

		// clang-format off
		expression.static_function("Expression static_zero(RHIShaderParameterType type)", &Expression::static_zero);
		expression.static_function("Expression static_half(RHIShaderParameterType type)", &Expression::static_half);
		expression.static_function("Expression static_one(RHIShaderParameterType type)", &Expression::static_one);
		expression.static_function("RHIShaderParameterType static_component_type_of(RHIShaderParameterType type)", &Expression::static_component_type_of);
		constexpr auto static_resolve1 = func_of<SPType, SPType, SPType>(&Expression::static_resolve);
		constexpr auto static_resolve2 = func_of<SPType, SPType, SPType, SPType>(&Expression::static_resolve);
		constexpr auto static_resolve3 = func_of<SPType, SPType, SPType, SPType, SPType>(&Expression::static_resolve);
		expression.static_function("RHIShaderParameterType static_resolve(RHIShaderParameterType type1, RHIShaderParameterType type2)", static_resolve1);
		expression.static_function("RHIShaderParameterType static_resolve(RHIShaderParameterType type1, RHIShaderParameterType type2, RHIShaderParameterType type3)", static_resolve2);
		expression.static_function("RHIShaderParameterType static_resolve(RHIShaderParameterType type1, RHIShaderParameterType type2, RHIShaderParameterType type3, RHIShaderParameterType type4)", static_resolve3);
		expression.static_function("string static_typename_of(RHIShaderParameterType type)", &Expression::static_typename_of);

		expression.static_function("RHIShaderParameterType static_make_float(RHIShaderParameterType self)", &Expression::static_make_float);
		expression.static_function("RHIShaderParameterType static_vector_clamp(RHIShaderParameterType self, uint8 min, uint8 max)", &Expression::static_vector_clamp);
		expression.static_function("RHIShaderParameterType static_make_vector(RHIShaderParameterType self, uint8 len)", &Expression::static_make_vector);
		expression.static_function("RHIShaderParameterType static_make_scalar(RHIShaderParameterType self)", &Expression::static_make_scalar);
		expression.static_function("bool static_is_scalar(RHIShaderParameterType self)", &Expression::static_is_scalar);
		expression.static_function("bool static_is_vector(RHIShaderParameterType self)", &Expression::static_is_vector);
		expression.static_function("bool static_is_matrix(RHIShaderParameterType self)", &Expression::static_is_matrix);
		expression.static_function("bool static_is_numeric(RHIShaderParameterType self)", &Expression::static_is_numeric);
		expression.static_function("bool static_is_meta(RHIShaderParameterType self)", &Expression::static_is_meta);
		expression.static_function("uint16 static_type_index(RHIShaderParameterType self)", &Expression::static_type_index);
		expression.static_function("uint8 static_vector_length(RHIShaderParameterType self)", &Expression::static_vector_length);

		// clang-format on

		// static RHIShaderParameterType component_type(RHIShaderParameterType type);
	}

	static ReflectionInitializeController init(reflection_init, "Engine::VisualMaterialGraph",
	                                           {"Engine::RHIShaderParameterType", "Engine::Refl::Class",
	                                            "Engine::VisualMaterialGraph::Node"});
}// namespace Engine::VisualMaterialGraph
