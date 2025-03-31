#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/set.hpp>
#include <Core/etl/vector.hpp>
#include <Core/object.hpp>
#include <Core/string_functions.hpp>

namespace Engine::VisualMaterialGraph
{
	class Pin;
	class OutputPin;
	class InputPin;
	class Node;

	class Expression
	{
	private:
		static const char* s_swizzle[4];

	public:
		ShaderParameterType type;
		String value;

		static Expression static_zero(ShaderParameterType type);
		static Expression static_half(ShaderParameterType type);
		static Expression static_one(ShaderParameterType type);
		static ShaderParameterType static_component_type_of(ShaderParameterType type);
		static ShaderParameterType static_resolve(ShaderParameterType type1, ShaderParameterType type2);
		static String static_typename_of(ShaderParameterType type);
		static Expression static_convert(const Expression& expression, ShaderParameterType type);
		static bool is_compatible_types(ShaderParameterType src, ShaderParameterType dst);


		inline Expression() : type(ShaderParameterType::Undefined) {}
		Expression(ShaderParameterType type, const char* value) : type(type), value(value) {}
		Expression(ShaderParameterType type, String&& value) : type(type), value(std::move(value)) {}
		Expression(ShaderParameterType type, const String& value) : type(type), value(value) {}

		explicit Expression(bool value) : type(ShaderParameterType::Bool), value(value ? "true" : "false") {}
		explicit Expression(float value) : type(ShaderParameterType::Float), value(Strings::format("{:.f}f", value)) {}
		explicit Expression(double value) : type(ShaderParameterType::Float), value(Strings::format("{:.f}f", value)) {}
		explicit Expression(int32_t value) : type(ShaderParameterType::Int), value(Strings::format("{}", value)) {}
		explicit Expression(uint32_t value) : type(ShaderParameterType::UInt), value(Strings::format("{}", value)) {}

		Expression x() const;
		Expression y() const;
		Expression z() const;
		Expression w() const;

		FORCE_INLINE Expression convert(ShaderParameterType type) const { return static_convert(*this, type); }
		FORCE_INLINE bool is_valid() const { return type != ShaderParameterType::Undefined; }
		FORCE_INLINE Expression& clear()
		{
			type = ShaderParameterType::Undefined;
			value.clear();
			return *this;
		}
	};

	class Compiler
	{
		Map<Pin*, Expression> m_expression;

		Vector<String> m_globals;
		Vector<String> m_locals;

		static String compile_expressions(const Vector<String>& expression, size_t tabs);

	public:
		Expression make_variable(const Expression& expression);
		Expression compile_default(Pin* pin);
		Expression compile(InputPin* pin);
		Expression compile(OutputPin* pin);

		String compile_global_expressions(size_t tabs = 0) const;
		String compile_local_expressions(size_t tabs = 1) const;
	};

	class Pin
	{
	public:
		struct DefaultValue {
			virtual byte* address()                  = 0;
			virtual ShaderParameterType type() const = 0;
			virtual Expression compile() const       = 0;
			virtual ~DefaultValue()                  = default;
		};

	private:
		String m_name;
		Node* m_node                  = nullptr;
		DefaultValue* m_default_value = nullptr;

		ShaderParameterType m_type = ShaderParameterType::Undefined;
		uint16_t m_index           = 0;

	public:
		enum Kind
		{
			Input,
			Output,
		};

		inline const String& name() const { return m_name; }
		inline Node* node() const { return m_node; }
		inline DefaultValue* default_value() const { return m_default_value; }
		inline ShaderParameterType type() const { return m_type; }
		inline uint16_t index() const { return m_index; }
		inline Identifier id() const { return reinterpret_cast<Identifier>(this); }

		virtual Pin& unlink() = 0;
		virtual inline OutputPin* as_output() { return nullptr; };
		virtual inline InputPin* as_input() { return nullptr; };
		virtual inline size_t links_count() const { return 0; }
		virtual inline Kind kind() const = 0;
		virtual ~Pin();

		friend class Node;
	};

	class OutputPin : public Pin
	{
	private:
		Set<InputPin*> m_links;

	public:
		OutputPin& link(InputPin* pin);
		OutputPin& unlink() override;

		inline OutputPin* as_output() override { return this; };
		inline size_t links_count() const override { return m_links.size(); }
		inline Kind kind() const override { return Kind::Output; }

		friend class InputPin;
		friend class Node;
	};

	class InputPin : public Pin
	{
	private:
		OutputPin* m_link = nullptr;

	public:
		InputPin& link(OutputPin* pin);
		InputPin& unlink() override;

		inline OutputPin* linked_pin() const { return m_link; }
		inline InputPin* as_input() override { return this; };
		inline size_t links_count() const override { return m_link ? 1 : 0; }
		inline Kind kind() const override { return Kind::Input; }

		friend class Pin;
		friend class Node;
	};

	class Node : public Object
	{
		trinex_declare_class(Node, Object);

	private:
		Vector<InputPin*> m_inputs;
		Vector<OutputPin*> m_outputs;

		Expression script_compile(OutputPin* pin, Compiler& compiler);

	public:
		template<typename T>
		struct Scriptable : public Super::Scriptable<T> {
			Expression compile(OutputPin* pin, Compiler& compiler) override { return Node::script_compile(pin, compiler); }
		};

	public:
		Vector2f position = {0.f, 0.f};

		InputPin* new_input(const String& name, ShaderParameterType type);
		OutputPin* new_output(const String& name, ShaderParameterType type);

		InputPin* new_input(const String& name, ShaderParameterType type, ShaderParameterType default_value_type);
		OutputPin* new_output(const String& name, ShaderParameterType type, ShaderParameterType default_value_type);

		virtual Expression compile(OutputPin* pin, Compiler& compiler);

		inline Identifier id() const { return reinterpret_cast<Identifier>(this); }
		inline Vector<InputPin*> inputs() const { return m_inputs; }
		inline Vector<OutputPin*> outputs() const { return m_outputs; }

		~Node();
	};

	class MaterialRoot : public Node
	{
		trinex_declare_class(MaterialRoot, Node);

	public:
		InputPin* const base_color;
		InputPin* const opacity;
		InputPin* const emissive;
		InputPin* const specular;
		InputPin* const metalness;
		InputPin* const roughness;
		InputPin* const ao;
		InputPin* const normal;
		InputPin* const position_offset;

		MaterialRoot();
	};
}// namespace Engine::VisualMaterialGraph
