#include <Core/class.hpp>
#include <Graphics/visual_material_graph.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_object.hpp>
#include <scriptarray.h>

namespace Engine::VisualMaterialGraph
{
	static ScriptFunction node_header_color;
	static ScriptFunction node_render;
	static ScriptFunction node_compile_out;
	static ScriptFunction node_compile_in;
	static ScriptFunction node_signature;
	static ScriptFunction signatured_node_make_expression;

	Vector4D Node::script_header_color() const
	{
		return *ScriptObject(this).execute(node_header_color).address_as<Vector4D>();
	}

	const NodeSignature& Node::script_signature() const
	{
		return **ScriptObject(this).execute(node_signature).address_as<NodeSignature*>();
	}

	Expression Node::script_compile(OutputPin* pin, CompilerState& state)
	{
		return *ScriptObject(this).execute(node_compile_out, pin, &state).address_as<Expression>();
	}

	Expression Node::script_compile(InputPin* pin, CompilerState& state)
	{
		return *ScriptObject(this).execute(node_compile_in, pin, &state).address_as<Expression>();
	}

	Node& Node::script_render()
	{
		ScriptObject(this).execute(node_render);
		return *this;
	}

	Node& Node::script_override_parameter(VisualMaterial* material)
	{
		return *this;
	}

	Expression SignaturedNode::script_make_expression(OutputPin* pin, const NodeSignature::Signature& signature,
	                                                  const Vector<Expression>& args)
	{
		return *ScriptObject(this).execute(signatured_node_make_expression, pin, &signature, &args).address_as<Expression>();
	}

	implement_class(Engine::VisualMaterialGraph, Node, Class::IsScriptable)
	{
		static_class_instance()->script_registration_callback = [](ScriptClassRegistrar* r, Class*) {
			node_header_color = r->method("Vector4D header_color() const", &This::scoped_header_color<This>);
			node_render       = r->method("Node@ render()", &This::scoped_render<This>);
			node_signature    = r->method("const NodeSignature& signature() const", &This::scoped_signature<This>);
			node_compile_out =
			        r->method("Expression compile(OutputPin@ pin, CompilerState@ state)", &This::scoped_compile_out<This>);
			node_compile_in =
			        r->method("Expression compile(InputPin@ pin, CompilerState@ state)", &This::scoped_compile_in<This>);

			r->method("bool can_connect(OutputPin@ pin, PinType type) final", &This::can_connect);
			r->method("bool is_root_node() const final", &This::is_root_node);
			r->method("bool has_error() const final", &This::has_error);
			r->method("const string& error_message() const final", &This::error_message);
			r->method("Node& clear_error_message() const final", &This::clear_error_message);
			r->method("void add_pin(InputPin@ pin) final", method_of<void, InputPin*>(&This::add_pin));
			r->method("void add_pin(OutputPin@ pin) final", method_of<void, OutputPin*>(&This::add_pin));
			r->method("InputPin@ input_pin(uint64 index) const final", &This::input_pin);
			r->method("OutputPin@ output_pin(uint64 index) const final", &This::output_pin);
			r->method("uint64 find_pin_index(OutputPin@ pin) const final", method_of<Index, OutputPin*>(&This::find_pin_index));
			r->method("uint64 find_pin_index(InputPin@ pin) const final", method_of<Index, InputPin*>(&This::find_pin_index));
			r->method("PinType in_pin_type(InputPin@ pin) const final", &This::in_pin_type);
			r->method("PinType out_pin_type(OutputPin@ pin) const final", &This::out_pin_type);
			r->method("uint64 id() const final", &This::id);
			r->property("Vector2D position", &This::position);
		};

		ScriptEngine::on_terminate.push([]() {
			node_header_color.release();
			node_render.release();
			node_compile_out.release();
			node_compile_in.release();
			node_signature.release();
		});
	}

	implement_class(Engine::VisualMaterialGraph, SignaturedNode, Class::IsScriptable)
	{
		static_class_instance()->script_registration_callback = [](ScriptClassRegistrar* r, Class*) {
			signatured_node_make_expression =
			        r->method("Expression make_expression(OutputPin@ pin, const NodeSignature::Signature& signature, const "
			                  "Vector<Expression>& args)",
			                  &This::scoped_make_expression<This>);

			r->method("Expression compile(OutputPin@ pin, CompilerState@ state)", &This::scoped_compile_out<This>);
		};

		ScriptEngine::on_terminate.push([]() { signatured_node_make_expression.release(); });
	}


	template<typename T>
	const char* typename_of()
	{
		return nullptr;
	}

#define declare_typename(type, name)                                                                                             \
	template<>                                                                                                                   \
	const char* typename_of<type>()                                                                                              \
	{                                                                                                                            \
		return #name;                                                                                                            \
	}

	declare_typename(bool, bool);
	declare_typename(int, int);
	declare_typename(uint_t, uint);
	declare_typename(float, float);
	declare_typename(BoolVector2D, BoolVector2D);
	declare_typename(BoolVector3D, BoolVector3D);
	declare_typename(BoolVector4D, BoolVector4D);
	declare_typename(IntVector2D, IntVector2D);
	declare_typename(IntVector3D, IntVector3D);
	declare_typename(IntVector4D, IntVector4D);
	declare_typename(UIntVector2D, UIntVector2D);
	declare_typename(UIntVector3D, UIntVector3D);
	declare_typename(UIntVector4D, UIntVector4D);
	declare_typename(Vector2D, Vector2D);
	declare_typename(Vector3D, Vector3D);
	declare_typename(Vector4D, Vector4D);
	declare_typename(Matrix3f, Matrix3f);
	declare_typename(Matrix4f, Matrix4f);

	template<typename, typename = std::void_t<>>
	struct pin_has_default_value : std::false_type {
	};

	template<typename T>
	struct pin_has_default_value<T, std::void_t<typename T::ValueType>> : std::true_type {
	};

	template<typename T, typename... Args>
	static T* factory_of(Args... args)
	{
		return new T(args...);
	}

	template<typename T>
	static T* implicit_cast(T* self)
	{
		return self;
	}

	template<typename Type>
	static inline void register_pin_type(const char* name)
	{
		auto info            = ScriptClassRegistrar::RefInfo();
		info.implicit_handle = true;
		info.no_count        = true;

		String full_name = "Engine::VisualMaterialGraph::";
		full_name += name;

		auto reg = ScriptClassRegistrar::reference_class(full_name, info);

		if constexpr (!std::is_same_v<Type, InputPin> && !std::is_same_v<Type, OutputPin>)
		{
			constexpr bool is_input_pin  = std::is_base_of_v<InputPin, Type>;
			constexpr bool is_output_pin = std::is_base_of_v<OutputPin, Type>;

			if constexpr (is_input_pin)
			{
				reg.method("InputPin@ opImplCast()", implicit_cast<Type>);
				reg.method("const InputPin@ opImplCast() const", implicit_cast<Type>);
			}
			else if constexpr (is_output_pin)
			{
				reg.method("OutputPin@ opImplCast()", implicit_cast<Type>);
				reg.method("const OutputPin@ opImplCast() const", implicit_cast<Type>);
			}
		}

		auto register_behaviour = [reg]() mutable {
			String factory_decl = Strings::format("{}@ f(Node node, const Engine::Name& name)", reg.class_base_name());
			reg.behave(ScriptClassBehave::Factory, factory_decl.c_str(), factory_of<Type, Node*, const Name&>,
			           ScriptCallConv::CDecl);

			if constexpr (pin_has_default_value<Type>::value)
			{
				if (const char* default_type_name = typename_of<typename Type::ValueType>())
				{
					String factory_decl = Strings::format("{}@ f(Node node, const Engine::Name& name, const {}& default_value)",
					                                      reg.class_base_name(), default_type_name);
					reg.behave(ScriptClassBehave::Factory, factory_decl.c_str(),
					           factory_of<Type, Node*, const Name&, const typename Type::ValueType&>, ScriptCallConv::CDecl);
				}
			}
		};

		ScriptBindingsInitializeController initializer(register_behaviour);
	}

	static Expression& expression_opAssign(Expression* self, const Expression& other)
	{
		*self = other;
		return *self;
	}

	static void register_expression_class()
	{
		auto info                    = ScriptClassRegistrar::ValueInfo();
		info.is_class                = true;
		info.more_constructors       = true;
		info.has_constructor         = true;
		info.has_destructor          = true;
		info.has_copy_constructor    = true;
		info.has_assignment_operator = true;

		auto reg = ScriptClassRegistrar::value_class("Engine::VisualMaterialGraph::Expression", sizeof(Expression), info);
		reg.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<Expression>);
		reg.behave(ScriptClassBehave::Construct, "void f(const Expression& expr)",
		           ScriptClassRegistrar::constructor<Expression, const Expression&>);
		reg.behave(ScriptClassBehave::Construct, "void f(const string& code, PinType type, bool is_var = false)",
		           ScriptClassRegistrar::constructor<Expression, const String&, PinType, bool>);
		reg.behave(ScriptClassBehave::Destruct, "void f()", ScriptClassRegistrar::destructor<Expression>);

		reg.method("Expression& opAssign(const Expression& other)", expression_opAssign);
		reg.method("bool is_valid() const", &Expression::is_valid);
		reg.method("Expression& reset()", &Expression::reset);

		reg.property("string code", &Expression::code);
		reg.property("PinType type", &Expression::type);
		reg.property("bool is_variable", &Expression::is_variable);
	}

	static void register_global_compiler_state()
	{
		auto info            = ScriptClassRegistrar::RefInfo();
		info.no_count        = true;
		info.implicit_handle = true;

		auto reg = ScriptClassRegistrar::reference_class("Engine::VisualMaterialGraph::GlobalCompilerState", info);

		reg.method("GlobalCompilerState& add(const string& expression, const string& name)", &GlobalCompilerState::add);
		reg.method("GlobalCompilerState& remove(const string& expression, const string& name)", &GlobalCompilerState::remove);
		reg.method("bool contains_expression(const string& expression) const", &GlobalCompilerState::contains_expression);
		reg.method("bool contains_name(const string& name) const", &GlobalCompilerState::contains_name);
		reg.method("uint64 globals_count() const", &GlobalCompilerState::globals_count);
	}

	static void register_compiler_state()
	{
		register_global_compiler_state();

		auto info            = ScriptClassRegistrar::RefInfo();
		info.no_count        = true;
		info.implicit_handle = true;

		auto reg = ScriptClassRegistrar::reference_class("Engine::VisualMaterialGraph::CompilerState", info);

		reg.property("GlobalCompilerState@ global_state", &CompilerState::global_state);

		reg.method("Expression create_variable(const Expression&)", &CompilerState::create_variable);
		reg.method("Expression expression_cast(const Expression& , PinType)", &CompilerState::expression_cast);
		reg.method("Expression pin_source(OutputPin@ pin)", method_of<Expression, OutputPin*>(&CompilerState::pin_source));
		reg.method("Expression pin_source(InputPin@ pin)", method_of<Expression, InputPin*>(&CompilerState::pin_source));
	}


	static void register_node_signatures()
	{
		auto info                    = ScriptClassRegistrar::ValueInfo();
		info.is_class                = true;
		info.has_constructor         = true;
		info.has_copy_constructor    = true;
		info.has_assignment_operator = true;
		info.has_destructor          = true;

		struct Helper {
			static Vector<PinType> copy_pin_type_array(CScriptArray* array)
			{
				Vector<PinType> result(array->GetSize());

				asUINT count = array->GetSize();

				for (asUINT i = 0; i < count; ++i)
				{
					result[i] = *reinterpret_cast<PinType*>(array->At(i));
				}
				return result;
			}

			static Set<PinType> copy_pin_type_set(CScriptArray* array)
			{
				Set<PinType> result(array->GetSize());

				asUINT count = array->GetSize();

				for (asUINT i = 0; i < count; ++i)
				{
					result.insert(*reinterpret_cast<PinType*>(array->At(i)));
				}
				return result;
			}

			static NodeSignature& add_signature(NodeSignature* self, CScriptArray* script_inputs, CScriptArray* script_outputs)
			{
				return self->add_signature(copy_pin_type_array(script_inputs), copy_pin_type_array(script_outputs));
			}

			static NodeSignature& add_input_types(NodeSignature* self, uint64_t index, CScriptArray* values)
			{
				return self->add_input_types(index, copy_pin_type_set(values));
			}
		};

		{
			using T = NodeSignature::Signature;
			auto reg =
			        ScriptClassRegistrar::value_class("Engine::VisualMaterialGraph::NodeSignature::Signature", sizeof(T), info);
			reg.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<T>);
			reg.behave(ScriptClassBehave::Construct, "void f(const Signature&)", ScriptClassRegistrar::constructor<T, const T&>);
			reg.behave(ScriptClassBehave::Destruct, "void f()", ScriptClassRegistrar::destructor<T>);
			reg.method("Signature& opAssign(const Signature&)", ScriptClassRegistrar::assign<T, const T&>);
			reg.method("uint64 inputs_count() const", &T::inputs_count);
			reg.method("uint64 outputs_count() const", &T::outputs_count);
			reg.method("PinType input(uint64 index) const", &T::input);
			reg.method("PinType output(uint64 index) const", &T::output);
		}

		{
			using T  = NodeSignature;
			auto reg = ScriptClassRegistrar::value_class("Engine::VisualMaterialGraph::NodeSignature", sizeof(T), info);
			reg.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<T>);
			reg.behave(ScriptClassBehave::Construct, "void f(const NodeSignature&)",
			           ScriptClassRegistrar::constructor<T, const T&>);
			reg.behave(ScriptClassBehave::Destruct, "void f()", ScriptClassRegistrar::destructor<T>);
			reg.method("NodeSignature& opAssign(const NodeSignature&)", ScriptClassRegistrar::assign<T, const T&>);
			reg.method("bool support_input_type(uint64 pin_index, PinType type) const", &T::support_input_type);
			reg.method("uint64 signatures_count() const", &T::signatures_count);
			reg.method("NodeSignature::Signature& signature(uint64 index) const", &T::signature);
			reg.method("NodeSignature& add_signature(const array<PinType>& inputs = {}, const array<PinType>& outputs = {})",
			           &Helper::add_signature);
			reg.method("NodeSignature& add_input_types(uint64, const array<PinType>& )", &Helper::add_input_types);
			reg.method("uint64 find_signature_index(Node@ node) const", &T::find_signature_index);
		}
	}

	static void initilize()
	{
		ScriptEnumRegistrar pin_type_enum("Engine::VisualMaterialGraph::PinType");
		pin_type_enum.set("Undefined", PinType::Undefined);

		pin_type_enum.set("Bool", PinType::Bool);
		pin_type_enum.set("Int", PinType::Int);
		pin_type_enum.set("UInt", PinType::UInt);
		pin_type_enum.set("Float", PinType::Float);
		pin_type_enum.set("BVec2", PinType::BVec2);
		pin_type_enum.set("IVec2", PinType::IVec2);
		pin_type_enum.set("UVec2", PinType::UVec2);
		pin_type_enum.set("Vec2", PinType::Vec2);
		pin_type_enum.set("BVec3", PinType::BVec3);
		pin_type_enum.set("IVec3", PinType::IVec3);
		pin_type_enum.set("UVec3", PinType::UVec3);
		pin_type_enum.set("Vec3", PinType::Vec3);
		pin_type_enum.set("Color3", PinType::Color3);
		pin_type_enum.set("BVec4", PinType::BVec4);
		pin_type_enum.set("IVec4", PinType::IVec4);
		pin_type_enum.set("UVec4", PinType::UVec4);
		pin_type_enum.set("Vec4", PinType::Vec4);
		pin_type_enum.set("Color4", PinType::Color4);
		pin_type_enum.set("Mat3", PinType::Mat3);
		pin_type_enum.set("Mat4", PinType::Mat4);
		pin_type_enum.set("Sampler", PinType::Sampler);
		pin_type_enum.set("Texture2D", PinType::Texture2D);

		register_pin_type<InputPin>("InputPin");
		register_pin_type<OutputPin>("OutputPin");

		register_pin_type<BoolInputPinND>("BoolInputPinND");
		register_pin_type<IntInputPinND>("IntInputPinND");
		register_pin_type<UIntInputPinND>("UIntInputPinND");
		register_pin_type<FloatInputPinND>("FloatInputPinND");
		register_pin_type<BVec2InputPinND>("BVec2InputPinND");
		register_pin_type<BVec3InputPinND>("BVec3InputPinND");
		register_pin_type<BVec4InputPinND>("BVec4InputPinND");
		register_pin_type<IVec2InputPinND>("IVec2InputPinND");
		register_pin_type<IVec3InputPinND>("IVec3InputPinND");
		register_pin_type<IVec4InputPinND>("IVec4InputPinND");
		register_pin_type<UVec2InputPinND>("UVec2InputPinND");
		register_pin_type<UVec3InputPinND>("UVec3InputPinND");
		register_pin_type<UVec4InputPinND>("UVec4InputPinND");
		register_pin_type<Vec2InputPinND>("Vec2InputPinND");
		register_pin_type<Vec3InputPinND>("Vec3InputPinND");
		register_pin_type<Color3InputPinND>("Color3InputPinND");
		register_pin_type<Vec4InputPinND>("Vec4InputPinND");
		register_pin_type<Color4InputPinND>("Color4InputPinND");
		register_pin_type<Mat3InputPinND>("Mat3InputPinND");
		register_pin_type<Mat4InputPinND>("Mat4InputPinND");
		register_pin_type<SamplerInputPinND>("SamplerInputPinND");
		register_pin_type<Texture2DInputPinND>("Texture2DInputPinND");

		register_pin_type<BoolInputPin>("BoolInputPin");
		register_pin_type<IntInputPin>("IntInputPin");
		register_pin_type<UIntInputPin>("UIntInputPin");
		register_pin_type<FloatInputPin>("FloatInputPin");
		register_pin_type<BVec2InputPin>("BVec2InputPin");
		register_pin_type<BVec3InputPin>("BVec3InputPin");
		register_pin_type<BVec4InputPin>("BVec4InputPin");
		register_pin_type<IVec2InputPin>("IVec2InputPin");
		register_pin_type<IVec3InputPin>("IVec3InputPin");
		register_pin_type<IVec4InputPin>("IVec4InputPin");
		register_pin_type<UVec2InputPin>("UVec2InputPin");
		register_pin_type<UVec3InputPin>("UVec3InputPin");
		register_pin_type<UVec4InputPin>("UVec4InputPin");
		register_pin_type<Vec2InputPin>("Vec2InputPin");
		register_pin_type<Vec3InputPin>("Vec3InputPin");
		register_pin_type<Color3InputPin>("Color3InputPin");
		register_pin_type<Vec4InputPin>("Vec4InputPin");
		register_pin_type<Color4InputPin>("Color4InputPin");
		register_pin_type<Mat3InputPin>("Mat3InputPin");
		register_pin_type<Mat4InputPin>("Mat4InputPin");
		register_pin_type<SamplerInputPin>("SamplerInputPin");
		register_pin_type<Texture2DInputPin>("Texture2DInputPin");

		register_pin_type<BoolOutputPinND>("BoolOutputPinND");
		register_pin_type<IntOutputPinND>("IntOutputPinND");
		register_pin_type<UIntOutputPinND>("UIntOutputPinND");
		register_pin_type<FloatOutputPinND>("FloatOutputPinND");
		register_pin_type<BVec2OutputPinND>("BVec2OutputPinND");
		register_pin_type<BVec3OutputPinND>("BVec3OutputPinND");
		register_pin_type<BVec4OutputPinND>("BVec4OutputPinND");
		register_pin_type<IVec2OutputPinND>("IVec2OutputPinND");
		register_pin_type<IVec3OutputPinND>("IVec3OutputPinND");
		register_pin_type<IVec4OutputPinND>("IVec4OutputPinND");
		register_pin_type<UVec2OutputPinND>("UVec2OutputPinND");
		register_pin_type<UVec3OutputPinND>("UVec3OutputPinND");
		register_pin_type<UVec4OutputPinND>("UVec4OutputPinND");
		register_pin_type<Vec2OutputPinND>("Vec2OutputPinND");
		register_pin_type<Vec3OutputPinND>("Vec3OutputPinND");
		register_pin_type<Color3OutputPinND>("Color3OutputPinND");
		register_pin_type<Vec4OutputPinND>("Vec4OutputPinND");
		register_pin_type<Color4OutputPinND>("Color4OutputPinND");
		register_pin_type<Mat3OutputPinND>("Mat3OutputPinND");
		register_pin_type<Mat4OutputPinND>("Mat4OutputPinND");
		register_pin_type<SamplerOutputPinND>("SamplerOutputPinND");
		register_pin_type<Texture2DOutputPinND>("Texture2DOutputPinND");

		register_pin_type<BoolOutputPin>("BoolOutputPin");
		register_pin_type<IntOutputPin>("IntOutputPin");
		register_pin_type<UIntOutputPin>("UIntOutputPin");
		register_pin_type<FloatOutputPin>("FloatOutputPin");
		register_pin_type<BVec2OutputPin>("BVec2OutputPin");
		register_pin_type<BVec3OutputPin>("BVec3OutputPin");
		register_pin_type<BVec4OutputPin>("BVec4OutputPin");
		register_pin_type<IVec2OutputPin>("IVec2OutputPin");
		register_pin_type<IVec3OutputPin>("IVec3OutputPin");
		register_pin_type<IVec4OutputPin>("IVec4OutputPin");
		register_pin_type<UVec2OutputPin>("UVec2OutputPin");
		register_pin_type<UVec3OutputPin>("UVec3OutputPin");
		register_pin_type<UVec4OutputPin>("UVec4OutputPin");
		register_pin_type<Vec2OutputPin>("Vec2OutputPin");
		register_pin_type<Vec3OutputPin>("Vec3OutputPin");
		register_pin_type<Color3OutputPin>("Color3OutputPin");
		register_pin_type<Vec4OutputPin>("Vec4OutputPin");
		register_pin_type<Color4OutputPin>("Color4OutputPin");
		register_pin_type<Mat3OutputPin>("Mat3OutputPin");
		register_pin_type<Mat4OutputPin>("Mat4OutputPin");
		register_pin_type<SamplerOutputPin>("SamplerOutputPin");
		register_pin_type<Texture2DOutputPin>("Texture2DOutputPin");


		register_expression_class();
		register_compiler_state();
		register_node_signatures();
	}

	static ReflectionInitializeController on_init(initilize, "Engine::VisualMaterialGraph::Bindings",
	                                              {"Engine::VisualMaterialGraph::Node"});

}// namespace Engine::VisualMaterialGraph
