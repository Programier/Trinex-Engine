#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/set.hpp>
#include <Core/etl/vector.hpp>
#include <Core/object.hpp>
#include <Core/string_functions.hpp>
#include <RHI/enums.hpp>

namespace Engine
{
	class VisualMaterial;
}

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
		RHIShaderParameterType type;
		String value;

		static Expression static_zero(RHIShaderParameterType type);
		static Expression static_half(RHIShaderParameterType type);
		static Expression static_one(RHIShaderParameterType type);
		static Expression static_convert(const Expression& expression, RHIShaderParameterType type);

		static RHIShaderParameterType static_component_type_of(RHIShaderParameterType type);
		static RHIShaderParameterType static_resolve(RHIShaderParameterType type1, RHIShaderParameterType type2);
		static RHIShaderParameterType static_resolve(RHIShaderParameterType type1, RHIShaderParameterType type2,
		                                             RHIShaderParameterType type3);
		static RHIShaderParameterType static_resolve(RHIShaderParameterType type1, RHIShaderParameterType type2,
		                                             RHIShaderParameterType type3, RHIShaderParameterType type4);
		static String static_typename_of(RHIShaderParameterType type);
		static bool is_compatible_types(RHIShaderParameterType src, RHIShaderParameterType dst);

		static RHIShaderParameterType static_make_float(RHIShaderParameterType self);
		static RHIShaderParameterType static_vector_clamp(RHIShaderParameterType self, byte min, byte max);
		static inline RHIShaderParameterType static_make_vector(RHIShaderParameterType self, byte len)
		{
			return self.make_vector(len);
		}
		static inline RHIShaderParameterType static_make_scalar(RHIShaderParameterType self) { return self.make_scalar(); }
		static inline bool static_is_scalar(RHIShaderParameterType self) { return self.is_scalar(); }
		static inline bool static_is_vector(RHIShaderParameterType self) { return self.is_vector(); }
		static inline bool static_is_matrix(RHIShaderParameterType self) { return self.is_matrix(); }
		static inline bool static_is_numeric(RHIShaderParameterType self) { return self.is_numeric(); }
		static inline bool static_is_meta(RHIShaderParameterType self) { return self.is_meta(); }
		static inline uint16_t static_type_index(RHIShaderParameterType self) { return self.type_index(); }
		static inline byte static_vector_length(RHIShaderParameterType self) { return self.vector_length(); }

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
		inline Expression to_floating() const { return convert(static_make_float(type)); }

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
	public:
		enum Stage
		{
			Vertex   = 0,
			Fragment = 1,
		};

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
		Vector<String> m_stage_locals[2];

		Node* m_current_node = nullptr;
		Stage m_stage        = Vertex;

		String next_var_name() const;

	public:
		Compiler();
		trinex_non_copyable(Compiler);
		trinex_non_moveable(Compiler);
		~Compiler();

		static String static_uniform_parameter_name(Node* node);
		static String static_uniform_parameter_name(Refl::Class* node_class, uint16_t id);

		Node* create_temp_node(Refl::Class* node_class, uint16_t id);

		template<typename T>
		inline T* create_temp_node(uint16_t id)
		{
			static_assert(std::is_base_of_v<Node, T>, "Node class must be derived from Node");
			return Object::instance_cast<T>(create_temp_node(T::static_class_instance(), id));
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

		String compile_includes(size_t tabs = 0) const;
		String compile_global_expressions(size_t tabs = 0) const;
		String compile_local_expressions(size_t tabs = 1) const;

		inline Compiler& stage(Stage stage)
		{
			m_stage = stage;
			return *this;
		}

		inline Stage stage() const { return m_stage; }
		inline bool is_vertex_stage() const { return m_stage == Vertex; }
		inline bool is_fragment_stage() const { return m_stage == Fragment; }
		inline Node* current_node() const { return m_current_node; }
	};

	class Pin
	{
	public:
		struct DefaultValue {
			virtual byte* address()                     = 0;
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
		uint16_t m_index              = 0;

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
		inline uint16_t index() const { return m_index; }

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
		uint16_t m_id = 0;

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
		virtual Node& change_id(uint16_t id);
		virtual Node& post_compile(VisualMaterial* material);

		inline uint16_t id() const { return m_id; }
		inline const Vector<InputPin*>& inputs() const { return m_inputs; }
		inline const Vector<OutputPin*>& outputs() const { return m_outputs; }

		~Node();
	};
}// namespace Engine::VisualMaterialGraph
