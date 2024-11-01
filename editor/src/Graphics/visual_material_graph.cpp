#include <Core/constants.hpp>
#include <Core/group.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/texture_2D.hpp>
#include <Graphics/visual_material.hpp>
#include <Graphics/visual_material_graph.hpp>

namespace Engine::VisualMaterialGraph
{
	namespace NodeBuilder
	{
		template<PinType... Args>
		struct input {
		};

		template<PinType... Args>
		struct output {
		};

		template<typename RetType, typename ArgsType>
		struct NodeFunction {
			using Ret  = RetType;
			using Args = ArgsType;
		};

		template<PinType... out, PinType... in>
		constexpr auto func(output<out...> = output<>(), input<in...> = input<>())
		{
			return NodeFunction<output<out...>, input<in...>>();
		}

		template<PinType... out, PinType... in>
		constexpr auto func(input<in...> = input<>(), output<out...> = output<>())
		{
			return NodeFunction<output<out...>, input<in...>>();
		}

		template<PinType... out, PinType... in>
		void register_function(NodeSignature* signature, NodeFunction<output<out...>, input<in...>>)
		{
			signature->add_signature({in...}, {out...});
		}

		template<auto... Functions>
		NodeSignature* make_signature()
		{
			static NodeSignature signature;
			return &signature;
		}

		template<auto... Functions>
		NodeSignature& build()
		{
			static NodeSignature* signature = nullptr;

			if (signature == nullptr)
			{
				signature = make_signature<Functions...>();
				(register_function(signature, Functions), ...);
			}

			return *signature;
		}

	}// namespace NodeBuilder

	NodeSignature::Signature::Signature(const Vector<PinType>& input, const Vector<PinType>& output)
	    : m_inputs(input), m_outputs(output)
	{}

	size_t NodeSignature::Signature::inputs_count() const
	{
		return m_inputs.size();
	}

	size_t NodeSignature::Signature::outputs_count() const
	{
		return m_outputs.size();
	}

	PinType NodeSignature::Signature::input(size_t index) const
	{
		if (index >= m_inputs.size())
			return PinType::Undefined;
		return m_inputs[index];
	}

	PinType NodeSignature::Signature::output(size_t index) const
	{
		if (index >= m_outputs.size())
			return PinType::Undefined;
		return m_outputs[index];
	}

	NodeSignature& NodeSignature::add_signature(const Vector<PinType>& input, const Vector<PinType>& output)
	{
		m_signatures.emplace_back(input, output);

		if (input.size() >= m_input_pin_types.size())
			m_input_pin_types.resize(input.size());

		int_t index = 0;

		for (auto& in : input)
		{
			m_input_pin_types[index].insert(in);
			++index;
		}
		return *this;
	}

	NodeSignature& NodeSignature::add_input_types(size_t index, const Set<PinType>& types)
	{
		if (index >= m_input_pin_types.size())
			m_input_pin_types.resize(index + 1);
		m_input_pin_types[index].insert(types.begin(), types.end());
		return *this;
	}

	int_t NodeSignature::find_signature_index(const Node* node) const
	{
		static auto calc_conv_cost = [](PinType src, PinType dst) -> int_t {
			if (src == dst)
				return 0;

			const TypeMetaData src_meta = src;
			const TypeMetaData dst_meta = dst;

			int_t cost = 1;

			if (src_meta.component_type != dst_meta.component_type)
			{
				++cost;
				if (src_meta.component_type_value > dst_meta.component_type_value)
					++cost;
			}

			if (src_meta.components != dst_meta.components)
			{
				auto min = glm::min(src_meta.components, dst_meta.components);
				auto max = glm::max(src_meta.components, dst_meta.components);
				cost += (max - min);

				if (src_meta.components > dst_meta.components)
					++cost;
			}
			else if (src_meta.components == dst_meta.components)
			{
				bool (*is_color)(const PinType&) = is_in<PinType::Color3, PinType::Color4>;

				if (is_color(dst))
					++cost;
			}

			return cost;
		};


		auto in_count  = node->inputs().size();
		auto out_count = node->outputs().size();

		int_t index = -1;
		int_t cost  = std::numeric_limits<int_t>::max();

		for (size_t i = 0, count = m_signatures.size(); i < count; ++i)
		{
			auto& signature = m_signatures[i];

			if (signature.inputs_count() != in_count || signature.outputs_count() != out_count)
				continue;

			int_t current_cost = 0;

			for (size_t input_index = 0, input_count = signature.inputs_count(); input_index < input_count; ++input_index)
			{
				PinType src = node->in_pin_type(node->inputs()[input_index]);
				PinType dst = signature.input(input_index);

				if (!is_convertable(src, dst))
				{
					current_cost = std::numeric_limits<int_t>::max();
					break;
				}

				current_cost += calc_conv_cost(src, dst);
			}

			if (current_cost < cost)
			{
				cost  = current_cost;
				index = static_cast<int_t>(i);
			}
		}

		return index;
	}

	bool NodeSignature::support_input_type(size_t pin_index, PinType type) const
	{
		if (pin_index >= m_input_pin_types.size())
			return false;
		return m_input_pin_types[pin_index].contains(type);
	}

	size_t NodeSignature::input_pin_types_count() const
	{
		size_t res = 0;
		for (auto& ell : m_input_pin_types)
		{
			res += ell.size();
		}
		return res;
	}

	size_t NodeSignature::signatures_count() const
	{
		return m_signatures.size();
	}

	const NodeSignature::Signature& NodeSignature::signature(size_t index) const
	{
		if (index >= m_signatures.size())
			throw EngineException("Index out of range!");
		return m_signatures[index];
	}

	Pin::Pin(Node* node, Name name) : m_node(node), m_name(name)
	{}

	Pin::~Pin()
	{}

	Node* Pin::node() const
	{
		return m_node;
	}

	Name Pin::name() const
	{
		return m_name;
	}

	bool Pin::has_links() const
	{
		return links_count() > 0;
	}

	InputPin::InputPin(Node* node, Name name) : Pin(node, name)
	{
		m_linked_to = nullptr;
	}

	PinType Pin::type() const
	{
		return PinType::Undefined;
	}

	void* Pin::default_value()
	{
		return nullptr;
	}


	PinKind InputPin::kind() const
	{
		return PinKind::Input;
	}

	size_t InputPin::links_count() const
	{
		return m_linked_to ? 1 : 0;
	}

	void InputPin::unlink()
	{
		if (m_linked_to)
		{
			m_linked_to->unlink_pin_internal(this);
			m_linked_to = nullptr;
		}
	}

	OutputPin* InputPin::linked_to() const
	{
		return m_linked_to;
	}

	void InputPin::create_link(OutputPin* pin)
	{
		unlink();
		pin->link_pin_internal(this);
		m_linked_to = pin;
	}

	OutputPin::OutputPin(Node* node, Name name) : Pin(node, name)
	{}

	PinKind OutputPin::kind() const
	{
		return PinKind::Output;
	}

	size_t OutputPin::links_count() const
	{
		return m_linked_to.size();
	}


	const Set<InputPin*>& OutputPin::linked_to() const
	{
		return m_linked_to;
	}

	void OutputPin::unlink()
	{
		Set<InputPin*> pins = std::move(m_linked_to);
		for (InputPin* pin : pins)
		{
			pin->unlink();
		}
	}

	void OutputPin::link_pin_internal(InputPin* pin)
	{
		pin->m_linked_to = this;
		m_linked_to.insert(pin);
	}

	void OutputPin::unlink_pin_internal(InputPin* pin)
	{
		m_linked_to.erase(pin);
	}

	void Node::add_pin(InputPin* pin)
	{
		m_inputs.push_back(pin);
	}

	void Node::add_pin(OutputPin* pin)
	{
		m_outputs.push_back(pin);
	}

	bool Node::is_root_node() const
	{
		return class_instance()->is_a(Root::static_class_instance());
	}

	Vector4D Node::header_color() const
	{
		return Vector4D(1.0, 0.0, 0.0, 1.0);
	}

	const NodeSignature& Node::signature() const
	{
		static NodeSignature sign;
		return sign;
	}

	Expression Node::compile(OutputPin* pin, CompilerState& state)
	{
		return Expression("", PinType::Undefined);
	}

	Expression Node::compile(InputPin* pin, CompilerState& state)
	{
		if (OutputPin* out = pin->linked_to())
		{
			return state.pin_source(out);
		}

		if (pin->default_value())
		{
			return Expression(create_default_value(pin->type(), pin->default_value()), pin->type());
		}
		return Expression();
	}

	Node& Node::render()
	{
		return *this;
	}

	Node& Node::override_parameter(VisualMaterial* material)
	{
		return *this;
	}

	bool Node::can_connect(InputPin* pin, PinType output_pin_type) const
	{
		auto src = pin->type();
		if (src == output_pin_type)
			return true;

		auto& node_signature = signature();
		return node_signature.support_input_type(find_pin_index(pin), output_pin_type);
	}

	PinType Node::in_pin_type(InputPin* pin) const
	{
		if (OutputPin* output_pin = pin->linked_to())
		{
			return output_pin->node()->out_pin_type(output_pin);
		}

		return pin->type();
	}

	PinType Node::out_pin_type(OutputPin* pin) const
	{
		auto& node_signature = signature();
		int_t index          = node_signature.find_signature_index(this);

		if (index != -1)
		{
			return node_signature.signature(index).output(find_pin_index(pin));
		}

		return pin->type();
	}

	const Vector<InputPin*>& Node::inputs() const
	{
		return m_inputs;
	}

	const Vector<OutputPin*>& Node::outputs() const
	{
		return m_outputs;
	}

	InputPin* Node::input_pin(Index index) const
	{
		if (index >= m_inputs.size())
			return nullptr;
		return m_inputs[index];
	}

	OutputPin* Node::output_pin(Index index) const
	{
		if (index >= m_outputs.size())
			return nullptr;
		return m_outputs[index];
	}

	Index Node::find_pin_index(OutputPin* pin) const
	{
		for (Index i = 0, count = m_outputs.size(); i < count; ++i)
		{
			if (m_outputs[i] == pin)
				return i;
		}

		return Constants::index_none;
	}

	Index Node::find_pin_index(InputPin* pin) const
	{
		for (Index i = 0, count = m_inputs.size(); i < count; ++i)
		{
			if (m_inputs[i] == pin)
				return i;
		}

		return Constants::index_none;
	}

	bool Node::has_error() const
	{
		return !m_error_message.empty();
	}

	const String& Node::error_message() const
	{
		return m_error_message;
	}

	Node& Node::clear_error_message()
	{
		m_error_message.clear();
		return *this;
	}

	bool Node::archive_process(Archive& ar)
	{
		if (!Super::archive_process(ar))
			return false;

		ar & position;

		for (auto input : m_inputs)
		{
			input->archive_process(ar);
		}

		for (auto output : m_outputs)
		{
			output->archive_process(ar);
		}

		return ar;
	}

	Node::~Node()
	{
		for (auto& input : m_inputs)
		{
			delete input;
		}

		for (auto& output : m_outputs)
		{
			delete output;
		}
	}

	const char* Node::name() const
	{
		return class_instance()->display_name().c_str();
	}

	Expression SignaturedNode::make_expression(OutputPin* pin, const NodeSignature::Signature& signature,
	                                           const Vector<Expression>& args)
	{
		return {};
	}

	Expression SignaturedNode::compile(OutputPin* pin, CompilerState& state)
	{
		auto& node_signature = signature();
		int index            = node_signature.find_signature_index(this);

		if (index == -1)
			return Expression();

		const NodeSignature::Signature& signature = node_signature.signature(index);

		Vector<Expression> arguments;
		int args_count = signature.inputs_count();
		arguments.resize(args_count);

		for (int i = 0; i < args_count; ++i)
		{
			Expression expr = state.pin_source(input_pin(i));
			expr            = state.expression_cast(expr, signature.input(i));

			if (!expr.is_valid())
				return Expression();

			arguments[i] = expr;
		}

		return make_expression(pin, signature, arguments);
	}

	String GlobalCompilerState::compile() const
	{
		String result = "";

		for (auto& entry : m_globals)
		{
			result += entry;
			result += ";\n";
		}
		return result;
	}

	GlobalCompilerState& GlobalCompilerState::add(const String& expression, const String& name)
	{
		m_globals.insert(expression);
		m_global_names.insert(name);
		return *this;
	}

	GlobalCompilerState& GlobalCompilerState::remove(const String& expression, const String& name)
	{
		m_globals.erase(expression);
		m_global_names.erase(name);
		return *this;
	}

	bool GlobalCompilerState::contains_expression(const String& expression) const
	{
		return m_globals.contains(expression);
	}

	bool GlobalCompilerState::contains_name(const String& name) const
	{
		return m_global_names.contains(name);
	}

	size_t GlobalCompilerState::globals_count() const
	{
		return m_global_names.size();
	}

	CompilerState::CompilerState(GlobalCompilerState& global_state) : global_state(&global_state)
	{}

	Expression CompilerState::create_variable(const Expression& in_expression)
	{
		if (in_expression.is_variable)
			return in_expression;

		String var_name = Strings::format("var_{}", m_locals.size());
		m_locals.push_back(Strings::format("{} {} = {};", slang_type_name(in_expression.type), var_name, in_expression.code));
		return Expression(var_name, in_expression.type, true);
	}

	Expression CompilerState::expression_cast(const Expression& in_expression, PinType out_type)
	{
		if (!is_convertable(in_expression.type, out_type))
			return Expression("", PinType::Undefined);

		TypeMetaData out_meta          = out_type;
		TypeMetaData in_expr_type_meta = in_expression.type;

		if (in_expression.type != out_type)
		{
			if (in_expr_type_meta.is_scalar)
			{
				if (out_meta.is_scalar)
				{
					return Expression(Strings::format("{}({})", slang_type_name(out_type), in_expression.code.c_str()), out_type);
				}

				if (out_meta.is_vector)
				{
					Expression variable = create_variable(in_expression);

					if (out_meta.components == 2)
						return Expression(Strings::format("{}({}, {})", slang_type_name(out_type), variable.code, variable.code),
						                  out_type);
					if (out_meta.components == 3)
						return Expression(Strings::format("{}({}, {}, {})", slang_type_name(out_type), variable.code,
						                                  variable.code, variable.code),
						                  out_type);
					if (out_meta.components == 4)
						return Expression(Strings::format("{}({}, {}, {}, {})", slang_type_name(out_type), variable.code,
						                                  variable.code, variable.code, variable.code),
						                  out_type);
				}
			}
			else if (in_expr_type_meta.is_vector)
			{
				if (out_meta.is_scalar)
				{
					return Expression(Strings::format("{}({}.x)", slang_type_name(out_type), in_expression.code.c_str()),
					                  out_type);
				}

				if (out_meta.is_vector)
				{
					Expression variable = create_variable(in_expression);

					PinType in_component_type  = in_expr_type_meta.component_type;
					PinType out_component_type = out_meta.component_type;

					if (in_component_type == out_component_type)
					{
						static const char* components_mask[] = {"xy", "xyz", "xyzw"};
						if (in_expr_type_meta.components > out_meta.components)
						{
							return Expression(Strings::format("{}({}.{})", slang_type_name(out_type), variable.code.c_str(),
							                                  components_mask[out_meta.components - 2]),
							                  out_type);
						}

						byte diff            = out_meta.components - in_expr_type_meta.components;
						String default_value = create_default_value(out_component_type, nullptr);

						if (diff == 1)
						{
							return Expression(Strings::format("{}({}.{}, {})", slang_type_name(out_type), variable.code.c_str(),
							                                  components_mask[in_expr_type_meta.components - 2], default_value),
							                  out_type);
						}
						else if (diff == 2)
						{
							return Expression(Strings::format("{}({}.{}, {}, {})", slang_type_name(out_type),
							                                  variable.code.c_str(),
							                                  components_mask[in_expr_type_meta.components - 2], default_value,
							                                  default_value),
							                  out_type);
						}
					}

					String default_value = create_default_value(out_component_type, nullptr);
					String code          = slang_type_name(out_type);
					code.push_back('(');

					for (byte i = 0; i < out_meta.components; ++i)
					{
						static constexpr const char* components_mask[] = {"x", "y", "z", "w"};

						if (i < in_expr_type_meta.components)
						{
							Expression component = expression_cast(
							        Expression(Strings::format("{}.{}", variable.code, components_mask[i]), in_component_type),
							        out_component_type);
							code += component.code;
						}
						else
						{
							code += default_value;
						}

						code += i == out_meta.components - 1 ? ")" : ", ";
					}

					return Expression(code, out_type, false);
				}
			}
			else if (in_expr_type_meta.is_matrix && out_meta.is_matrix)
			{
				if (out_meta.components < in_expr_type_meta.components)
				{
					return Expression(Strings::format("{}({})", slang_type_name(out_type), in_expression.code), out_type);
				}
				else
				{
					throw EngineException("Unimplemented matrix cast");
				}
			}
		}
		else
		{
			return in_expression;
		}

		return Expression("", PinType::Undefined);
	}

	String CompilerState::create_header(const char* prefix) const
	{
		String result = {};

		for (auto& line : m_locals)
		{
			result += Strings::format("{}{}\n", prefix, line);
		}
		return result;
	}

	Expression CompilerState::pin_source(OutputPin* pin)
	{
		Expression& expression = m_internal_cache[pin->id()];

		if (!expression.is_valid())
		{
			expression = pin->node()->compile(pin, *this);
			if (pin->links_count() > 1)
			{
				expression = create_variable(expression);
			}
		}
		return expression;
	}

	Expression CompilerState::pin_source(InputPin* pin)
	{
		return pin->node()->compile(pin, *this);
	}

	const char* slang_type_name(PinType type)
	{

		switch (type)
		{
			case PinType::Bool:
				return "bool";
			case PinType::Int:
				return "int";
			case PinType::UInt:
				return "uint";
			case PinType::Float:
				return "float";
			case PinType::BVec2:
				return "bool2";
			case PinType::BVec3:
				return "bool3";
			case PinType::BVec4:
				return "bool4";
			case PinType::IVec2:
				return "int2";
			case PinType::IVec3:
				return "int3";
			case PinType::IVec4:
				return "int4";
			case PinType::UVec2:
				return "uint2";
			case PinType::UVec3:
				return "uint3";
			case PinType::UVec4:
				return "uint4";
			case PinType::Vec2:
				return "float2";
			case PinType::Vec3:
			case PinType::Color3:
				return "float3";
			case PinType::Vec4:
			case PinType::Color4:
				return "float4";
			case PinType::Mat3:
				return "float3x3";
			case PinType::Mat4:
				return "float4x4";
			case PinType::Sampler:
				return "Sampler";
			case PinType::Texture2D:
				return "Sampler2D";
			default:
				break;
		}
		return "Undefined";
	}

	String create_default_value(PinType type, const void* data)
	{
		union Storage
		{
			const void* ptr;
			const bool* bool_ptr;
			const int_t* int_ptr;
			const uint_t* uint_ptr;
			const float* float_ptr;

			Storage(const void* ptr) : ptr(ptr)
			{}
		} storage(data);

		static auto bool_to_string = [](bool v) -> const char* { return v ? "true" : "false"; };

		switch (type)
		{
			case PinType::Bool:
				return storage.bool_ptr && storage.bool_ptr[0] ? "true" : "false";
			case PinType::Int:
				return storage.ptr ? Strings::format("{}", storage.int_ptr[0]) : "0";
			case PinType::UInt:
				return storage.ptr ? Strings::format("{}", storage.uint_ptr[0]) : "0";
			case PinType::Float:
				return storage.ptr ? Strings::format("{:.6f}f", storage.float_ptr[0]) : "0.f";
			case PinType::BVec2:
				return storage.ptr ? Strings::format("bool2({}, {})", bool_to_string(storage.bool_ptr[0]),
				                                     bool_to_string(storage.bool_ptr[1]))
				                   : "bool2(false, false)";
			case PinType::BVec3:
				return storage.ptr ? Strings::format("bool3({}, {}, {})", bool_to_string(storage.bool_ptr[0]),
				                                     bool_to_string(storage.bool_ptr[1]), bool_to_string(storage.bool_ptr[2]))
				                   : "bool3(false, false, false)";
			case PinType::BVec4:
				return storage.ptr ? Strings::format("bool4({}, {}, {}, {})", bool_to_string(storage.bool_ptr[0]),
				                                     bool_to_string(storage.bool_ptr[1]), bool_to_string(storage.bool_ptr[2]),
				                                     bool_to_string(storage.bool_ptr[3]))
				                   : "bool4(false, false, false, false)";
			case PinType::IVec2:
				return storage.ptr ? Strings::format("int2({}, {})", storage.int_ptr[0], storage.int_ptr[1]) : "int2(0, 0)";
			case PinType::IVec3:
				return storage.ptr
				               ? Strings::format("int3({}, {}, {})", storage.int_ptr[0], storage.int_ptr[1], storage.int_ptr[2])
				               : "int3(0, 0, 0)";
			case PinType::IVec4:
				return storage.ptr ? Strings::format("int4({}, {}, {}, {})", storage.int_ptr[0], storage.int_ptr[1],
				                                     storage.int_ptr[2], storage.int_ptr[3])
				                   : "int4(0, 0, 0, 0)";
			case PinType::UVec2:
				return storage.ptr ? Strings::format("uint2({}, {})", storage.uint_ptr[0], storage.uint_ptr[1]) : "uint2(0, 0)";
			case PinType::UVec3:
				return storage.ptr ? Strings::format("uint3({}, {}, {})", storage.uint_ptr[0], storage.uint_ptr[1],
				                                     storage.uint_ptr[2])
				                   : "uint3(0, 0, 0)";
			case PinType::UVec4:
				return storage.ptr ? Strings::format("uint4({}, {}, {}, {})", storage.uint_ptr[0], storage.uint_ptr[1],
				                                     storage.uint_ptr[2], storage.uint_ptr[3])
				                   : "uint4(0, 0, 0, 0)";

			case PinType::Vec2:
				return storage.float_ptr ? Strings::format("float2({:.6f}, {:.6f})", storage.float_ptr[0], storage.float_ptr[1])
				                         : "float2(0.f, 0.f)";
			case PinType::Vec3:
			case PinType::Color3:
				return storage.float_ptr ? Strings::format("float3({:.6f}, {:.6f}, {:.6f})", storage.float_ptr[0],
				                                           storage.float_ptr[1], storage.float_ptr[2])
				                         : "float3(0.f, 0.f, 0.f)";
			case PinType::Vec4:
			case PinType::Color4:
				return storage.float_ptr ? Strings::format("float4({:.6f}, {:.6f}, {:.6f}, {:.6f})", storage.float_ptr[0],
				                                           storage.float_ptr[1], storage.float_ptr[2], storage.float_ptr[3])
				                         : "float4(0.f, 0.f, 0.f, 0.f)";

			default:
				return "";
		}

		return "";
	}


	//////////////// NODES IMPLEMENTATION ////////////////

	implement_visual_material_node(Root, Init);

	Root::Root()
	{
		m_inputs.push_back(new Color3InputPin(this, "Base Color"));
		m_inputs.push_back(new FloatInputPin(this, "Opacity", 1.f));
		m_inputs.push_back(new Color3InputPin(this, "Emissive"));
		m_inputs.push_back(new FloatInputPin(this, "Specular"));
		m_inputs.push_back(new FloatInputPin(this, "Metalness"));
		m_inputs.push_back(new FloatInputPin(this, "Roughness"));
		m_inputs.push_back(new FloatInputPin(this, "AO", 1.f));
		m_inputs.push_back(new Vec3InputPinND(this, "Normal"));
		m_inputs.push_back(new Vec3InputPin(this, "Position Offset"));
	}

	Expression Root::compile(InputPin* pin, CompilerState& state)
	{
		Index index = find_pin_index(pin);
		Expression expression;

		if (index == 7 && !m_inputs[7]->has_links())
		{
			expression = Expression("input.world_normal", PinType::Vec3, true);
		}

		if (!expression.is_valid())
		{
			expression = Node::compile(pin, state);
		}

		return state.expression_cast(expression, pin->type());
	}

	const NodeSignature& Root::signature() const
	{
		static NodeSignature root;
		static bool is_not_initialized = true;

		if (is_not_initialized)
		{
			is_not_initialized = false;

			using enum PinType;
			root.add_input_types(0, {Bool, Int, UInt, Float, BVec2, IVec2, UVec2, Vec2, BVec3, IVec3, UVec3, Vec3, Color3, BVec4,
			                         IVec4, UVec4, Vec4, Color4});
		}

		return root;
	}

	////////////////////////// CONSTANTS BLOCK //////////////////////////

#define implement_visual_material_constant_node(type)                                                                            \
	implement_visual_material_node(type, Constants);                                                                             \
	type::type()                                                                                                                 \
	{                                                                                                                            \
		m_outputs.push_back(new type##OutputPin(this, "Value"));                                                                 \
	}                                                                                                                            \
	Expression type::compile(OutputPin* pin, CompilerState& state)                                                               \
	{                                                                                                                            \
		return Expression(create_default_value(PinType::type, pin->default_value()), PinType::type);                             \
	}

	implement_visual_material_constant_node(Bool);
	implement_visual_material_constant_node(Int);
	implement_visual_material_constant_node(UInt);
	implement_visual_material_constant_node(Float);
	implement_visual_material_constant_node(BVec2);
	implement_visual_material_constant_node(BVec3);
	implement_visual_material_constant_node(BVec4);
	implement_visual_material_constant_node(IVec2);
	implement_visual_material_constant_node(IVec3);
	implement_visual_material_constant_node(IVec4);
	implement_visual_material_constant_node(UVec2);
	implement_visual_material_constant_node(UVec3);
	implement_visual_material_constant_node(UVec4);
	implement_visual_material_constant_node(Vec2);
	implement_visual_material_constant_node(Vec3);
	implement_visual_material_constant_node(Vec4);
	implement_visual_material_constant_node(Color3);
	implement_visual_material_constant_node(Color4);


	////////////////////////// INPUTS BLOCK //////////////////////////

#define implement_visual_material_input_node(name, type, expr)                                                                   \
	implement_visual_material_node(name, Inputs);                                                                                \
	name::name()                                                                                                                 \
	{                                                                                                                            \
		m_outputs.push_back(new type##OutputPinND(this, "Value"));                                                               \
	}                                                                                                                            \
	Expression name::compile(OutputPin* pin, CompilerState& state)                                                               \
	{                                                                                                                            \
		return Expression(expr, PinType::type, true);                                                                            \
	}

	implement_visual_material_input_node(Projection, Mat4, "globals.projection");
	implement_visual_material_input_node(View, Mat4, "globals.view");
	implement_visual_material_input_node(ProjView, Mat4, "globals.projview");
	implement_visual_material_input_node(InvProjView, Mat4, "globals.inv_projview");
	implement_visual_material_input_node(Viewport, Vec4, "globals.viewport");
	implement_visual_material_input_node(CameraLocation, Vec3, "globals.camera_location");
	implement_visual_material_input_node(CameraForward, Vec3, "globals.camera_forward");
	implement_visual_material_input_node(CameraRight, Vec3, "globals.camera_right");
	implement_visual_material_input_node(CameraUp, Vec3, "globals.camera_up");
	implement_visual_material_input_node(CameraProjectionMode, Int, "globals.camera_projection_mode");
	implement_visual_material_input_node(Size, Vec2, "globals.size");
	implement_visual_material_input_node(DepthRange, Vec2, "globals.depth_range");
	implement_visual_material_input_node(Gamma, Float, "globals.gamma");
	implement_visual_material_input_node(Time, Float, "globals.time");
	implement_visual_material_input_node(DeltaTime, Float, "globals.delta_time");
	implement_visual_material_input_node(FOV, Float, "globals.fov");
	implement_visual_material_input_node(OrthoWidth, Float, "globals.ortho_width");
	implement_visual_material_input_node(OrthoHeight, Float, "globals.ortho_height");
	implement_visual_material_input_node(NearClipPlane, Float, "globals.near_clip_plane");
	implement_visual_material_input_node(FarClipPlane, Float, "globals.far_clip_plane");
	implement_visual_material_input_node(AspectRatio, Float, "globals.aspect_ratio");

	implement_visual_material_node(UV, Inputs);

	UV::UV()
	{
		m_outputs.push_back(new Vec2OutputPinND(this, "UV"));
	}

	Expression UV::compile(OutputPin* pin, CompilerState& state)
	{
		String expression = Strings::format("input.uv[{}]", m_index);
		return Expression(expression, PinType::Vec2, true);
	}

	UV& UV::render()
	{
		Super::render();
		ImGui::SetNextItemWidth(100.f);
		if (ImGui::InputInt("Index", &m_index, 1))
		{
			m_index = glm::clamp<int_t>(m_index, 0, 7);
		}
		return *this;
	}


	////////////////////////// MATH BLOCK //////////////////////////
	implement_visual_material_node(Abs, Math);
	implement_visual_material_node(Add, Math);
	implement_visual_material_node(Sub, Math);
	implement_visual_material_node(Mul, Math);
	implement_visual_material_node(Div, Math);
	implement_visual_material_node(Sin, Math);
	implement_visual_material_node(Cos, Math);

	implement_class_default_init(Engine::VisualMaterialGraph::BinaryOperatorNode, 0);

	const NodeSignature& BinaryOperatorNode::signature() const
	{
		using enum PinType;
		using namespace NodeBuilder;
		constexpr auto a = func(output<Int>{}, input<Int, Int>{});
		constexpr auto b = func(output<UInt>{}, input<UInt, UInt>{});
		constexpr auto c = func(output<Float>{}, input<Float, Float>{});
		constexpr auto d = func(output<IVec2>{}, input<IVec2, IVec2>{});
		constexpr auto e = func(output<UVec2>{}, input<UVec2, UVec2>{});
		constexpr auto f = func(output<Vec2>{}, input<Vec2, Vec2>{});
		constexpr auto g = func(output<IVec3>{}, input<IVec3, IVec3>{});
		constexpr auto h = func(output<UVec3>{}, input<UVec3, UVec3>{});
		constexpr auto k = func(output<Vec3>{}, input<Vec3, Vec3>{});
		constexpr auto l = func(output<Color3>{}, input<Color3, Color3>{});
		constexpr auto m = func(output<IVec4>{}, input<IVec4, IVec4>{});
		constexpr auto n = func(output<UVec4>{}, input<UVec4, UVec4>{});
		constexpr auto o = func(output<Vec4>{}, input<Vec4, Vec4>{});
		constexpr auto p = func(output<Color4>{}, input<Color4, Color4>{});
		return build<a, b, c, d, e, f, g, h, k, l, m, n, o, p>();
	}

	Abs::Abs()
	{
		m_inputs.push_back(new FloatInputPin(this, "In"));
		m_outputs.push_back(new FloatOutputPinND(this, "Out"));
	}


	Expression Abs::compile(OutputPin* pin, CompilerState& state)
	{
		Expression expression  = state.pin_source(m_inputs[0]);
		expression             = state.expression_cast(expression, to_int_or_float(expression.type));
		expression.code        = Strings::format("abs({})", expression.code);
		expression.is_variable = false;
		return expression;
	}

	Add::Add()
	{
		m_inputs.push_back(new FloatInputPin(this, "A"));
		m_inputs.push_back(new FloatInputPin(this, "B"));
		m_outputs.push_back(new FloatOutputPinND(this, "Out"));
	}

	Sub::Sub()
	{
		m_inputs.push_back(new FloatInputPin(this, "A"));
		m_inputs.push_back(new FloatInputPin(this, "B"));
		m_outputs.push_back(new FloatOutputPinND(this, "Out"));
	}

	Mul::Mul()
	{
		m_inputs.push_back(new FloatInputPin(this, "A"));
		m_inputs.push_back(new FloatInputPin(this, "B"));
		m_outputs.push_back(new FloatOutputPinND(this, "Out"));
	}

	Div::Div()
	{
		m_inputs.push_back(new FloatInputPin(this, "A"));
		m_inputs.push_back(new FloatInputPin(this, "B"));
		m_outputs.push_back(new FloatOutputPinND(this, "Out"));
	}

	Sin::Sin()
	{
		m_inputs.push_back(new FloatInputPin(this, "In"));
		m_outputs.push_back(new FloatOutputPinND(this, "Out"));
	}

	Expression Sin::compile(OutputPin* pin, CompilerState& state)
	{
		Expression expression = state.pin_source(m_inputs[0]);
		expression            = state.expression_cast(expression, to_floating_point(expression.type));

		if (!expression.is_valid())
		{
			m_error_message = "Input expression is invalid!";
			return {};
		}

		expression.code        = Strings::format("sin({})", expression.code);
		expression.is_variable = false;
		return expression;
	}


	Cos::Cos()
	{
		m_inputs.push_back(new FloatInputPin(this, "In"));
		m_outputs.push_back(new FloatOutputPinND(this, "Out"));
	}


	Expression Cos::compile(OutputPin* pin, CompilerState& state)
	{
		Expression expression = state.pin_source(m_inputs[0]);
		expression            = state.expression_cast(expression, to_floating_point(expression.type));

		if (!expression.is_valid())
		{
			m_error_message = "Input expression is invalid!";
			return {};
		}

		expression.code        = Strings::format("cos({})", expression.code);
		expression.is_variable = false;
		return expression;
	}

	////////////////////////// COMMON BLOCK //////////////////////////

	// implement_visual_material_node(ComponentMask, Common);

	// ComponentMask::ComponentMask()
	// {
	// 	m_inputs.push_back(new InputPin(this, "In"));
	// 	m_outputs.push_back(new OutputPin(this, "Out"));
	// }

	// Expression ComponentMask::compile(OutputPin* pin, CompilerState& state)
	// {
	// 	if (!m_inputs[0]->has_links())
	// 		return Expression();

	// 	auto input_pin        = m_inputs[0];
	// 	Expression input_expr = state.create_variable(input_pin->linked_to()->node()->compile(input_pin->linked_to(), state));
	// 	PinType out_type      = out_pin_type(pin);

	// 	if (out_type == PinType::Undefined)
	// 		return Expression();

	// 	if (TypeMetaData(out_type).is_vector)
	// 	{
	// 		const char mask[4] = {'r', 'g', 'b', 'a'};
	// 		String code        = Strings::format("{}.", input_expr.code);

	// 		byte count = TypeMetaData(input_pin->linked_to()->node()->out_pin_type(input_pin->linked_to())).components;

	// 		for (byte i = 0; i < count; ++i)
	// 		{
	// 			if (masks[i])
	// 			{
	// 				code += mask[i];
	// 			}
	// 		}

	// 		return Expression(code, out_type, false);
	// 	}

	// 	if (out_type == input_expr.type)
	// 		return input_expr;
	// 	return Expression();
	// }

	// bool ComponentMask::can_connect(InputPin* pin, PinType linked_pin_type)
	// {
	// 	return true;
	// }

	// PinType ComponentMask::out_pin_type(OutputPin* pin)
	// {
	// 	if (!m_inputs[0]->has_links())
	// 		return PinType::Undefined;
	// 	auto linked_pin = m_inputs[0]->linked_to();

	// 	PinType input_type = linked_pin->node()->out_pin_type(linked_pin);

	// 	TypeMetaData input_meta = input_type;
	// 	if (input_meta.is_vector)
	// 	{
	// 		size_t components        = input_meta.components;
	// 		PinType component_type   = input_meta.component_type;
	// 		byte new_type_components = 0;

	// 		for (size_t i = 0; i < components; ++i)
	// 		{
	// 			if (masks[i])
	// 			{
	// 				new_type_components += 1;
	// 			}
	// 		}

	// 		return construct_vector_type(component_type, new_type_components);
	// 	}

	// 	return input_type;
	// }

	// ComponentMask& ComponentMask::render()
	// {
	// 	Super::render();

	// 	static const char* names[4] = {
	// 	        "R/X",
	// 	        "G/Y",
	// 	        "B/Z",
	// 	        "A/W",
	// 	};

	// 	for (int i = 0; i < 4; ++i)
	// 	{
	// 		ImGui::Checkbox(names[i], &masks[i]);
	// 	}

	// 	return *this;
	// }

	////////////////////////// TEXTURES BLOCK //////////////////////////

	implement_visual_material_node(Sampler, Textures);
	implement_visual_material_node(Texture2D, Textures);

	Sampler::Sampler()
	{
		m_outputs.push_back(new SamplerOutputPin(this, "Sampler", DefaultResources::Samplers::default_sampler));
	}

	Texture2D::Texture2D()
	    : texture(DefaultResources::Textures::default_texture), sampler(DefaultResources::Samplers::default_sampler)
	{
		m_inputs.push_back(new SamplerInputPin(this, "Sampler", DefaultResources::Samplers::default_sampler));
		m_inputs.push_back(new IntInputPinND(this, "LOD"));
		m_inputs.push_back(new Vec2InputPinND(this, "UV"));

		m_outputs.push_back(new Color4OutputPinND(this, "Color"));
		m_outputs.push_back(new FloatOutputPinND(this, "R"));
		m_outputs.push_back(new FloatOutputPinND(this, "G"));
		m_outputs.push_back(new FloatOutputPinND(this, "B"));
		m_outputs.push_back(new FloatOutputPinND(this, "A"));
	}

	Texture2D& Texture2D::render()
	{
		ImGui::SetNextItemWidth(125.f);
		ImGui::InputText("Name", m_name);
		ImGui::Image(texture.ptr(), {125, 125});

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ContentBrowser->Object");
			if (payload)
			{
				IM_ASSERT(payload->DataSize == sizeof(Object*));

				Object* new_object = *reinterpret_cast<Object**>(payload->Data);

				if (Engine::Texture2D* new_texture = Object::instance_cast<Engine::Texture2D>(new_object))
				{
					texture = new_texture;
				}
			}
			ImGui::EndDragDropTarget();
		}
		return *this;
	}

	Expression Texture2D::compile(OutputPin* pin, CompilerState& state)
	{
		Index pin_index = find_pin_index(pin);
		if (pin_index == Constants::index_none)
			return {};

		auto& cache = state.cache[id()];
		String sampled_image;
		bool sampled_image_is_variable = true;

		if (cache.has_value())
		{
			sampled_image = cache.cast<String>();
		}
		else
		{
			if (m_name.empty())
			{
				m_error_message = "Parameter name can't be empty!";
				return {};
			}

			if (state.global_state->contains_name(m_name))
			{
				m_error_message = Strings::format("Parameter with name '{}' already exist!", m_name);
				return {};
			}

			state.global_state->add(Strings::format("Sampler2D {}", m_name), m_name);
			InputPin* UV = m_inputs[2];

			Expression expression;
			if (OutputPin* pin = UV->linked_to())
			{
				expression = state.expression_cast(pin->node()->compile(pin, state), PinType::Vec2);
			}
			else
			{
				expression = Expression("input.uv[0]", PinType::Vec2, false);
			}
			sampled_image = Strings::format("{}.Sample({})", m_name, expression.code);

			size_t links = 0;
			for (auto& output : m_outputs)
			{
				links += output->links_count();
			}

			if (links > 1)
			{
				sampled_image = state.create_variable(Expression(sampled_image, PinType::Color4, false)).code;
			}
			else
			{
				sampled_image_is_variable = false;
			}

			// Find sampler instance
			sampler = nullptr;

			if (m_inputs[0]->has_links())
			{
				Expression expression = m_inputs[0]->node()->compile(m_inputs[0]->linked_to(), state);
				if (expression.is_valid())
				{
					sampler = reinterpret_cast<Engine::Sampler*>(expression.userdata);
				}
			}

			if (sampler == nullptr)
				sampler = DefaultResources::Samplers::default_sampler;
		}

		static auto create_swizzle = [](const String& code, char component) -> Expression {
			return Expression(Strings::format("{}.{}", code, component), PinType::Float, false);
		};

		switch (pin_index)
		{
			case 0:
				return Expression(sampled_image, PinType::Color4, sampled_image_is_variable);
			case 1:
				return create_swizzle(sampled_image, 'r');
			case 2:
				return create_swizzle(sampled_image, 'g');
			case 3:
				return create_swizzle(sampled_image, 'b');
			case 4:
				return create_swizzle(sampled_image, 'a');
			default:
				return {};
		}

		return {};
	}

	Texture2D& Texture2D::override_parameter(VisualMaterial* material)
	{
		Super::override_parameter(material);

		if (auto texture_parameter = material->find_parameter<MaterialParameters::Sampler2D>(m_name))
		{
			texture_parameter->texture = texture == nullptr ? DefaultResources::Textures::default_texture : texture.ptr();
			texture_parameter->sampler = sampler;

			if (texture_parameter->sampler == nullptr)
				texture_parameter->sampler = DefaultResources::Samplers::default_sampler;
		}

		return *this;
	}
}// namespace Engine::VisualMaterialGraph
