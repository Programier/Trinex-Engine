#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/set.hpp>
#include <Core/etl/vector.hpp>
#include <Core/object.hpp>
#include <Core/string_functions.hpp>
#include <RHI/enums.hpp>

namespace Trinex
{
	class VisualMaterial;
}

namespace Trinex::VisualMaterialGraph
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
		RHIShaderParameterType type;
		String value;

		static Expression make_zero(RHIShaderParameterType type);
		static Expression make_half(RHIShaderParameterType type);
		static Expression make_one(RHIShaderParameterType type);
		static Expression convert(const Expression& expression, RHIShaderParameterType type);

		static RHIShaderParameterType component_type_of(RHIShaderParameterType type);
		static RHIShaderParameterType resolve(RHIShaderParameterType type1, RHIShaderParameterType type2);
		static RHIShaderParameterType resolve(RHIShaderParameterType type1, RHIShaderParameterType type2,
		                                      RHIShaderParameterType type3);
		static RHIShaderParameterType resolve(RHIShaderParameterType type1, RHIShaderParameterType type2,
		                                      RHIShaderParameterType type3, RHIShaderParameterType type4);
		static String typename_of(RHIShaderParameterType type);
		static bool is_compatible_types(RHIShaderParameterType src, RHIShaderParameterType dst);

		static RHIShaderParameterType make_floating(RHIShaderParameterType self);
		static RHIShaderParameterType vector_clamp(RHIShaderParameterType self, u8 min, u8 max);

		ENGINE_EXPORT static RHIShaderParameterType make_numeric(RHIShaderParameterType base, u8 len = 1)
		{
			return RHIShaderParameterType::make_numeric(base, len);
		}

		ENGINE_EXPORT static RHIShaderParameterType make_matrix(RHIShaderParameterType base, u8 rows = 4, u8 columns = 4)
		{
			return RHIShaderParameterType::make_matrix(base, rows, columns);
		}

		inline Expression() : type(RHIShaderParameterType::Undefined) {}
		Expression(RHIShaderParameterType type, const char* value) : type(type), value(value) {}
		Expression(RHIShaderParameterType type, String&& value) : type(type), value(std::move(value)) {}
		Expression(RHIShaderParameterType type, const String& value) : type(type), value(value) {}

		Expression x() const;
		Expression y() const;
		Expression z() const;
		Expression w() const;

		Expression convert(RHIShaderParameterType dst) const;
		Expression vector_length() const;
		inline Expression to_floating() const { return convert(make_floating(type)); }

		FORCE_INLINE bool is_valid() const { return type != RHIShaderParameterType::Undefined; }
		FORCE_INLINE Expression& clear()
		{
			type = RHIShaderParameterType::Undefined;
			value.clear();
			return *this;
		}
	};

	class Compiler
	{
	private:
		struct RedirectionKey {
			Refl::Class* node_class;
			Identifier id;

			inline bool operator<(const RedirectionKey& other) const
			{
				if (node_class != other.node_class)
					return node_class < other.node_class;
				return id < other.id;
			}

			inline bool operator==(const RedirectionKey& other) const { return node_class == other.node_class && id == other.id; }
			inline bool operator!=(const RedirectionKey& other) const { return !(*this == other); }
		};

		TreeMap<RedirectionKey, Node*> m_redirections;
		Map<Pin*, Expression> m_expression;

		Set<String> m_includes;
		Set<String> m_globals;
		Set<String> m_param_names;
		Set<String> m_var_names;

		Set<Node*> m_temp_nodes;
		Vector<String> m_locals;

		Node* m_current_node = nullptr;
		String next_var_name() const;

	public:
		Compiler();
		trinex_non_copyable(Compiler);
		trinex_non_moveable(Compiler);
		~Compiler();

		static String static_uniform_parameter_name(Node* node);
		static String static_uniform_parameter_name(Refl::Class* node_class, u16 id);

		Node* create_temp_node(Refl::Class* node_class, u16 id);

		template<typename T>
		inline T* create_temp_node(u16 id)
		{
			static_assert(std::is_base_of_v<Node, T>, "Node class must be derived from Node");
			return Object::instance_cast<T>(create_temp_node(T::static_reflection(), id));
		}

		Compiler& add_redirection(Node* node, Identifier id);
		Node* find_redirection(Refl::Class* node_class, Identifier id);

		Compiler& add_include(const StringView& include);
		Expression make_uniform(RHIShaderParameterType type, const String& name_override = "");
		Expression make_variable(RHIShaderParameterType type);
		Expression make_variable(const Expression& expression);
		Expression compile_default(Pin* pin);
		Expression compile(InputPin* pin);
		Expression compile(OutputPin* pin);

		String compile_includes(usize tabs = 0) const;
		String compile_global_expressions(usize tabs = 0) const;
		String compile_local_expressions(usize tabs = 1) const;
		inline Node* current_node() const { return m_current_node; }
	};

	class Pin
	{
	public:
		struct DefaultValue {
			virtual u8* address()                       = 0;
			virtual RHIShaderParameterType type() const = 0;
			virtual Expression compile() const          = 0;
			virtual ~DefaultValue()                     = default;

			template<typename T>
			inline T& ref()
			{
				return *reinterpret_cast<T*>(address());
			}
		};

	private:
		String m_name;
		Node* m_node                  = nullptr;
		DefaultValue* m_default_value = nullptr;

		RHIShaderParameterType m_type = RHIShaderParameterType::Undefined;
		u16 m_index                   = 0;

	public:
		enum Kind
		{
			Input,
			Output,
		};

		inline const String& name() const { return m_name; }
		inline Node* node() const { return m_node; }
		inline DefaultValue* default_value() const { return m_default_value; }
		inline RHIShaderParameterType type() const { return m_type; }
		inline u16 index() const { return m_index; }

		virtual Pin& unlink() = 0;
		virtual inline OutputPin* as_output() { return nullptr; };
		virtual inline InputPin* as_input() { return nullptr; };
		virtual inline usize links_count() const { return 0; }
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
		inline usize links_count() const override { return m_links.size(); }
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
		inline usize links_count() const override { return m_link ? 1 : 0; }
		inline Kind kind() const override { return Kind::Input; }

		friend class Pin;
		friend class Node;
	};

	class Node : public Object
	{
		trinex_class(Node, Object);

	private:
		Vector<InputPin*> m_inputs;
		Vector<OutputPin*> m_outputs;
		u16 m_id = 0;

		Expression script_compile(OutputPin* pin, Compiler& compiler);
		void script_render();

	public:
		template<typename T>
		struct Scriptable : public Super::Scriptable<T> {
			Expression compile(OutputPin* pin, Compiler& compiler) override { return Node::script_compile(pin, compiler); }
			Scriptable& render() override
			{
				Node::script_render();
				return *this;
			}
		};

	public:
		Vector2f position = {0.f, 0.f};

		static void static_node_group(Refl::Class* node_class, const String& group);
		InputPin* new_input(const String& name, RHIShaderParameterType type);
		OutputPin* new_output(const String& name, RHIShaderParameterType type);

		InputPin* new_input(const String& name, RHIShaderParameterType type, RHIShaderParameterType default_value_type);
		OutputPin* new_output(const String& name, RHIShaderParameterType type, RHIShaderParameterType default_value_type);

		Node& on_property_changed(const Refl::PropertyChangedEvent& event) override;

		virtual Expression compile(OutputPin* pin, Compiler& compiler);
		virtual Node& render();
		virtual Node& change_id(u16 id);
		virtual Node& post_compile(VisualMaterial* material);

		inline u16 id() const { return m_id; }
		inline const Vector<InputPin*>& inputs() const { return m_inputs; }
		inline const Vector<OutputPin*>& outputs() const { return m_outputs; }

		~Node();
	};
}// namespace Trinex::VisualMaterialGraph
