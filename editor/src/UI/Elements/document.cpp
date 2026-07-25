#include "../internal.hpp"
#include <Core/etl/string.hpp>
#include <Core/etl/utility.hpp>
#include <Core/etl/variant.hpp>
#include <Core/etl/vector.hpp>
#include <Core/file_manager.hpp>
#include <UI/Elements/document.hpp>
#include <UI/reflection.hpp>
#include <peglib.h>

namespace Trinex::UI
{
	namespace Markup
	{
		inline constexpr const char* grammar = R"(
Document <- DocumentItem* ~EndOfFile

DocumentItem <- Include / Style / Node
Include      <- 'include' String ';'?
Style        <- 'style' StyleSelector '{' StyleMember* '}'
StyleSelector <- Identifier PseudoClass*
PseudoClass   <- ':' Identifier
StyleMember   <- TransitionBlock ';'? / Property
TransitionBlock <- 'transition' ':'? '{' TransitionProperty* '}'
TransitionProperty <- Identifier (':' TransitionArgs)? ';'?
TransitionArgs <- Value TransitionEase? TransitionDelay?
TransitionEase <- ',' Identifier
TransitionDelay <- ',' Value

Node           <- Identifier '{' Member* '}'
Member         <- PropertyMember / NodeMember
PropertyMember <- Property
NodeMember     <- Node
Property       <- Identifier ':' Value ';'?

Value <- Localization
       / Binding
       / String
       / Float
       / Integer
       / Boolean
       / Null
       / List
       / IdentifierValue

Localization <- < '@' Identifier ('.' Identifier)* >
Binding      <- '$' Identifier ('.' Identifier)* BindingMode?
BindingMode  <- ':' < 'rw' / 'wr' / 'r' / 'w' >

List <- '[' (Value (',' Value)*)? ']'

String     <- < '"' (Escape / StringChar)* '"' > { no_whitespace }
Escape     <- '\\' ['"\\nrt]
StringChar <- !['"\\] .

Float   <- < [+-]? ([0-9]+ '.' [0-9]* / '.' [0-9]+ / [0-9]+ [eE] [+-]? [0-9]+) ([eE] [+-]? [0-9]+)? >
Integer <- < [+-]? [0-9]+ >
Boolean <- < 'true' / 'false' >
Null    <- < 'null' >

IdentifierValue <- Identifier
Identifier      <- < [A-Za-z_][A-Za-z0-9_]* >

EndOfFile <- !.

%whitespace <- (_Space / _Comment)*
_Space      <- [ \t\r\n]
_Comment    <- '//' (![\r\n] .)*
	)";

		struct Property;
		struct Node;

		struct Property {
			String name;
			ValueDesc value;
			SourceLocation location;
		};

		struct Include {
			Path path;
			SourceLocation location;
		};

		struct TransitionBlock {
			Vector<StyleTransition> transitions;
			SourceLocation location;
		};

		struct Style {
			StyleSelector selector;
			Vector<StyleProperty> properties;
			Vector<StyleTransition> transitions;
			SourceLocation location;
		};

		struct Node {
			String type;
			Vector<Property> properties;
			Vector<Node> children;
			SourceLocation location;
		};

		using Member       = Variant<Property, Node>;
		using StyleMember  = Variant<Property, TransitionBlock>;
		using DocumentItem = Variant<Include, Style, Node>;
		using StyleMap     = StyleSheet;

		namespace Detail
		{
			inline SourceLocation location_of(const peg::SemanticValues& values)
			{
				const auto [line, column] = values.line_info();
				return {.line = static_cast<u32>(line), .column = static_cast<u32>(column)};
			}

			inline String unescape_string(StringView token)
			{
				if (token.size() < 2 || token.front() != '"' || token.back() != '"')
				{
					throw std::runtime_error("Invalid quoted string");
				}

				String result;
				for (std::size_t index = 1; index + 1 < token.size(); ++index)
				{
					const char character = token[index];

					if (character != '\\')
					{
						result += character;
						continue;
					}

					++index;

					if (index >= token.size() - 1)
					{
						throw std::runtime_error("Incomplete escape sequence");
					}

					switch (token[index])
					{
						case '"': result += '"'; break;
						case '\\': result += '\\'; break;
						case 'n': result += '\n'; break;
						case 'r': result += '\r'; break;
						case 't': result += '\t'; break;
						default: throw std::runtime_error("Unsupported escape sequence");
					}
				}

				return result;
			}

			template<typename T>
			const T& any_ref(const std::any& value)
			{
				return std::any_cast<const T&>(value);
			}

			inline String join_strings(const std::vector<std::string>& values)
			{
				String result;

				for (usize index = 0; index < values.size(); ++index)
				{
					if (index != 0)
					{
						result += ", ";
					}

					result += values[index].c_str();
				}

				return result;
			}

			inline StyleProperty to_style_property(const Property& property)
			{
				return StyleProperty{
				        .name     = property.name,
				        .value    = property.value,
				        .location = property.location,
				};
			}

			inline StyleState state_of(const Name& pseudo_class)
			{
				if (pseudo_class == "hover")
				{
					return StyleState::Hover;
				}

				if (pseudo_class == "active")
				{
					return StyleState::Active;
				}

				if (pseudo_class == "focus" || pseudo_class == "focused")
				{
					return StyleState::Focus;
				}

				if (pseudo_class == "disabled")
				{
					return StyleState::Disabled;
				}

				return StyleState::Undefined;
			}

			inline bool value_to_f32(f32& dst, const ValueDesc& src)
			{
				if (const auto* value = etl::get_if<f32>(&src.value))
				{
					dst = *value;
					return true;
				}

				if (const auto* value = etl::get_if<i32>(&src.value))
				{
					dst = static_cast<f32>(*value);
					return true;
				}

				return false;
			}

			inline Ease ease_of(const Name& name)
			{
				if (name == "Linear")
					return Ease::Linear;
				if (name == "InQuad")
					return Ease::InQuad;
				if (name == "OutQuad")
					return Ease::OutQuad;
				if (name == "InOutQuad")
					return Ease::InOutQuad;
				if (name == "InCubic")
					return Ease::InCubic;
				if (name == "OutCubic")
					return Ease::OutCubic;
				if (name == "InOutCubic")
					return Ease::InOutCubic;
				if (name == "InExpo")
					return Ease::InExpo;
				if (name == "OutExpo")
					return Ease::OutExpo;
				if (name == "InOutExpo")
					return Ease::InOutExpo;
				if (name == "OutBack")
					return Ease::OutBack;

				return Ease::OutCubic;
			}
		}// namespace Detail

		class Parser
		{
		private:
			peg::parser m_parser;

		private:
			Parser() : m_parser(grammar)
			{
				configure_logger();
				configure_actions();
			}

			void configure_logger()
			{
				m_parser.set_error_reporter([](const peg::ErrorReport& report) {
					String expected;

					if (!report.expected_literals.empty())
					{
						expected += " literals [";
						expected += Detail::join_strings(report.expected_literals);
						expected += "]";
					}

					if (!report.expected_rules.empty())
					{
						if (!expected.empty())
						{
							expected += ",";
						}

						expected += " rules [";
						expected += Detail::join_strings(report.expected_rules);
						expected += "]";
					}

					const char* token = report.unexpected_token.empty() ? "<eof>" : report.unexpected_token.c_str();

					if (expected.empty())
					{
						trinex_error(Log::Editor, "Markup parse error at %zu:%zu near '%s': %s", report.line, report.col, token,
						             report.message.c_str());
					}
					else
					{
						trinex_error(Log::Editor, "Markup parse error at %zu:%zu near '%s': expected%s. %s", report.line,
						             report.col, token, expected.c_str(), report.message.c_str());
					}
				});
			}

			void configure_actions()
			{
				configure_leaf_actions();
				configure_value_actions();
				configure_tree_actions();
			}

			void configure_leaf_actions()
			{
				m_parser["Identifier"] = [](const peg::SemanticValues& values) { return Identifier{values.token()}; };

				m_parser["IdentifierValue"] = [](const peg::SemanticValues& values) {
					const auto& identifier = Detail::any_ref<Identifier>(values[0]);

					return ValueDesc{
					        .value    = Value{identifier},
					        .location = Detail::location_of(values),
					};
				};

				m_parser["Localization"] = [](const peg::SemanticValues& values) {
					LocalizationKey key;
					key.reserve(values.size());

					for (std::size_t index = 0; index < values.size(); ++index)
					{
						key.emplace_back(Detail::any_ref<Identifier>(values[index]));
					}

					return ValueDesc{
					        .value    = Value{key},
					        .location = Detail::location_of(values),
					};
				};

				m_parser["Binding"] = [](const peg::SemanticValues& values) {
					BindingPath binding;
					binding.reserve(values.size());

					for (std::size_t index = 0; index < values.size(); ++index)
					{
						if (const auto* mode = std::any_cast<BindingPath::Mode>(&values[index]))
						{
							binding.mode = *mode;
						}
						else
						{
							binding.emplace_back(Detail::any_ref<Identifier>(values[index]));
						}
					}

					return ValueDesc{
					        .value    = Value{binding},
					        .location = Detail::location_of(values),
					};
				};

				m_parser["BindingMode"] = [](const peg::SemanticValues& values) -> BindingPath::Mode {
					StringView token = values.token();

					if (token == "w")
						return BindingPath::Mode::Write;

					if (token == "rw" || token == "wr")
						return BindingPath::Mode::RW;

					return BindingPath::Mode::Read;
				};

				m_parser["String"] = [](const peg::SemanticValues& values) {
					return ValueDesc{
					        .value    = Value{Detail::unescape_string(values.token())},
					        .location = Detail::location_of(values),
					};
				};

				m_parser["Integer"] = [](const peg::SemanticValues& values) {
					return ValueDesc{
					        .value    = Value{values.token_to_number<i32>()},
					        .location = Detail::location_of(values),
					};
				};

				m_parser["Float"] = [](const peg::SemanticValues& values) {
					return ValueDesc{
					        .value    = Value{values.token_to_number<f32>()},
					        .location = Detail::location_of(values),
					};
				};

				m_parser["Boolean"] = [](const peg::SemanticValues& values) {
					return ValueDesc{
					        .value    = Value{values.token() == "true"},
					        .location = Detail::location_of(values),
					};
				};

				m_parser["Null"] = [](const peg::SemanticValues& values) {
					return ValueDesc{.value = Value{Null{}}, .location = Detail::location_of(values)};
				};
			}

			void configure_value_actions()
			{
				m_parser["List"] = [](const peg::SemanticValues& values) {
					Container list;
					list.reserve(values.size());

					for (const std::any& item : values)
					{
						list.emplace_back(Detail::any_ref<ValueDesc>(item));
					}

					return ValueDesc{.value = std::move(list), .location = Detail::location_of(values)};
				};

				m_parser["Value"]  = [](const peg::SemanticValues& values) { return Detail::any_ref<ValueDesc>(values[0]); };
				m_parser["Member"] = [](const peg::SemanticValues& values) -> std::any { return values[0]; };
			}

			void configure_tree_actions()
			{
				m_parser["Property"] = [](const peg::SemanticValues& values) {
					const auto& identifier = Detail::any_ref<Identifier>(values[0]);

					return Property{
					        .name     = identifier,
					        .value    = Detail::any_ref<ValueDesc>(values[1]),
					        .location = Detail::location_of(values),
					};
				};

				m_parser["PropertyMember"] = [](const peg::SemanticValues& values) {
					return Member{Detail::any_ref<Property>(values[0])};
				};

				m_parser["NodeMember"] = [](const peg::SemanticValues& values) {
					return Member{Detail::any_ref<Node>(values[0])};
				};

				m_parser["Member"] = [](const peg::SemanticValues& values) { return Detail::any_ref<Member>(values[0]); };

				m_parser["Node"] = [](const peg::SemanticValues& values) {
					const auto& identifier = Detail::any_ref<Identifier>(values[0]);

					Node node{
					        .type       = identifier,
					        .properties = {},
					        .children   = {},
					        .location   = Detail::location_of(values),
					};

					for (usize index = 1; index < values.size(); ++index)
					{
						const auto& member = Detail::any_ref<Member>(values[index]);

						auto visitor = [&node](const auto& value) {
							using Type = std::remove_cvref_t<decltype(value)>;

							if constexpr (std::is_same_v<Type, Property>)
							{
								node.properties.push_back(value);
							}
							else if constexpr (std::is_same_v<Type, Node>)
							{
								node.children.push_back(value);
							}
						};

						etl::visit(visitor, member);
					}

					return node;
				};

				m_parser["TransitionEase"] = [](const peg::SemanticValues& values) {
					return Detail::any_ref<Identifier>(values[0]);
				};

				m_parser["TransitionDelay"] = [](const peg::SemanticValues& values) {
					return Detail::any_ref<ValueDesc>(values[0]);
				};

				m_parser["TransitionProperty"] = [](const peg::SemanticValues& values) {
					StyleTransition transition;
					transition.property = Detail::any_ref<Identifier>(values[0]);

					bool has_duration = false;

					for (usize index = 1; index < values.size(); ++index)
					{
						if (const auto* value = std::any_cast<ValueDesc>(&values[index]))
						{
							if (!has_duration)
							{
								Detail::value_to_f32(transition.duration, *value);
								has_duration = true;
							}
							else
							{
								Detail::value_to_f32(transition.delay, *value);
							}
						}
						else if (const auto* ease = std::any_cast<Identifier>(&values[index]))
						{
							transition.ease = Detail::ease_of(*ease);
						}
					}

					return transition;
				};

				m_parser["TransitionBlock"] = [](const peg::SemanticValues& values) {
					TransitionBlock block;
					block.location = Detail::location_of(values);
					block.transitions.reserve(values.size());

					for (const std::any& value : values)
					{
						block.transitions.push_back(Detail::any_ref<StyleTransition>(value));
					}

					return block;
				};

				m_parser["StyleMember"] = [](const peg::SemanticValues& values) -> std::any {
					if (const auto* transition = std::any_cast<TransitionBlock>(&values[0]))
					{
						return StyleMember{*transition};
					}

					return StyleMember{Detail::any_ref<Property>(values[0])};
				};

				m_parser["PseudoClass"] = [](const peg::SemanticValues& values) {
					return Detail::any_ref<Identifier>(values[0]);
				};

				m_parser["StyleSelector"] = [](const peg::SemanticValues& values) {
					StyleSelector selector;
					selector.name        = Detail::any_ref<Identifier>(values[0]);
					selector.specificity = 1;

					for (usize index = 1; index < values.size(); ++index)
					{
						const Name pseudo_class = Detail::any_ref<Identifier>(values[index]);
						const StyleState state  = Detail::state_of(pseudo_class);

						if (state != StyleState::Undefined)
						{
							selector.states |= state;
							selector.specificity += 10;
						}
						else
						{
							trinex_error(Log::Editor, "Unknown style pseudo-class '%s' at %u:%u", pseudo_class.c_str(),
							             Detail::location_of(values).line, Detail::location_of(values).column);
						}
					}

					return selector;
				};

				m_parser["Include"] = [](const peg::SemanticValues& values) {
					const auto& value  = Detail::any_ref<ValueDesc>(values[0]);
					const String* path = etl::get_if<String>(&value.value);

					return Include{
					        .path     = path == nullptr ? Path() : Path(*path),
					        .location = Detail::location_of(values),
					};
				};

				m_parser["Style"] = [](const peg::SemanticValues& values) {
					Style style{
					        .selector    = Detail::any_ref<StyleSelector>(values[0]),
					        .properties  = {},
					        .transitions = {},
					        .location    = Detail::location_of(values),
					};

					style.properties.reserve(values.size() - 1);

					for (usize index = 1; index < values.size(); ++index)
					{
						const auto& member = Detail::any_ref<StyleMember>(values[index]);

						auto visitor = [&style](const auto& value) {
							using Type = std::remove_cvref_t<decltype(value)>;

							if constexpr (std::is_same_v<Type, Property>)
							{
								style.properties.push_back(Detail::to_style_property(value));
							}
							else if constexpr (std::is_same_v<Type, TransitionBlock>)
							{
								for (const StyleTransition& transition : value.transitions)
								{
									style.transitions.push_back(transition);
								}
							}
						};

						etl::visit(visitor, member);
					}

					return style;
				};

				m_parser["DocumentItem"] = [](const peg::SemanticValues& values) -> std::any {
					if (const auto* include = std::any_cast<Include>(&values[0]))
					{
						return DocumentItem{*include};
					}

					if (const auto* style = std::any_cast<Style>(&values[0]))
					{
						return DocumentItem{*style};
					}

					return DocumentItem{Detail::any_ref<Node>(values[0])};
				};

				m_parser["Document"] = [](const peg::SemanticValues& values) {
					Vector<DocumentItem> items;
					items.reserve(values.size());

					for (const std::any& value : values)
					{
						items.emplace_back(Detail::any_ref<DocumentItem>(value));
					}

					return items;
				};
			}

		public:
			static Parser* instance()
			{
				static thread_local Parser parser;
				return &parser;
			}

			bool parse(Vector<DocumentItem>& items, StringView source)
			{
				if (source.empty())
				{
					return false;
				}

				Vector<DocumentItem> result;

				const bool success = m_parser.parse(source, result);

				if (!success)
				{
					return false;
				}

				items = etl::move(result);
				return true;
			}
		};

		static Path resolve_include_path(const Path& owner_path, const Path& include_path)
		{
			if (owner_path.empty() || include_path.starts_with("[") || include_path.starts_with("/"))
			{
				return include_path;
			}

			return Path(owner_path.base_path()) / include_path;
		}

		static bool value_to_name(Name& name, const ValueDesc& desc)
		{
			if (const auto* identifier = etl::get_if<Identifier>(&desc.value))
			{
				name = *identifier;
				return true;
			}

			if (const auto* string = etl::get_if<String>(&desc.value))
			{
				name = *string;
				return true;
			}

			return false;
		}

		static bool append_style_names(Vector<Name>& styles, const ValueDesc& desc)
		{
			Name name;

			if (value_to_name(name, desc))
			{
				styles.push_back(name);
				return true;
			}

			if (const auto* list = etl::get_if<Container>(&desc.value))
			{
				for (const ValueDesc& item : *list)
				{
					if (!append_style_names(styles, item))
					{
						return false;
					}
				}

				return true;
			}

			return false;
		}

		static bool apply_property(Element* element, const Node& node, const Property& prop)
		{
			auto type = element->type();

			auto visitor = [&]<typename T>(const T& value) -> bool {
				if (!type->assign(element, prop.name, &value, UI::Refl::NativeType<T>::instance()))
				{
					trinex_error(Log::Editor, "Failed to assign property '%s' of element '%s' at %u:%u", prop.name.c_str(),
					             node.type.c_str(), prop.location.line, prop.location.column);
					return false;
				}

				return true;
			};

			return etl::visit(visitor, prop.value.value);
		}

		static bool apply_properties(Element* element, const Node& node, const Vector<Property>& properties)
		{
			for (const Property& prop : properties)
			{
				if (!apply_property(element, node, prop))
				{
					return false;
				}
			}

			return true;
		}

		static bool create_elements(Document* document, Element* owner, const Node& node)
		{
			auto element = owner->attach(node.type);

			if (element == nullptr)
			{
				trinex_error(Log::Editor, "Failed to create element '%s' at %u:%u", node.type.c_str(), node.location.line,
				             node.location.column);
				return false;
			}

			Vector<Name> style_names;
			Name element_id;

			for (const Property& prop : node.properties)
			{
				if (prop.name == "class" || prop.name == "style")
				{
					if (!append_style_names(style_names, prop.value))
					{
						trinex_error(Log::Editor, "Invalid style reference on element '%s' at %u:%u", node.type.c_str(),
						             prop.location.line, prop.location.column);
						return false;
					}
				}
				else if (prop.name == "id")
				{
					value_to_name(element_id, prop.value);
				}
			}

			for (const Name& style_name : style_names)
			{
				element->style(style_name);
			}

			for (const Property& prop : node.properties)
			{
				if (prop.name == "class" || prop.name == "style")
				{
					continue;
				}

				element->inline_property(Detail::to_style_property(prop));

				if (prop.name == "id" && !apply_property(element, node, prop))
				{
					return false;
				}
			}

			if (element_id.is_valid())
			{
				document->register_element(element_id, element);
			}

			for (const Node& child : node.children)
			{
				if (!create_elements(document, element, child))
				{
					return false;
				}
			}

			return true;
		}

		static bool load_items(Vector<Node>& nodes, StyleMap& styles, StringView source, const Path& path, u32 depth)
		{
			if (depth > 32)
			{
				trinex_error(Log::Editor, "Markup include depth exceeded while loading '%s'", path.c_str());
				return false;
			}

			auto parser = Parser::instance();
			Vector<DocumentItem> items;

			if (!parser->parse(items, source))
			{
				return false;
			}

			for (const DocumentItem& item : items)
			{
				auto visitor = [&](const auto& value) -> bool {
					using Type = std::remove_cvref_t<decltype(value)>;

					if constexpr (std::is_same_v<Type, Include>)
					{
						const Path include_path = resolve_include_path(path, value.path);
						FileReader reader(include_path);

						if (!reader.is_open())
						{
							trinex_error(Log::Editor, "Failed to include UI document '%s' at %u:%u", include_path.c_str(),
							             value.location.line, value.location.column);
							return false;
						}

						return load_items(nodes, styles, reader.read_string(), include_path, depth + 1);
					}
					else if constexpr (std::is_same_v<Type, Style>)
					{
						styles.add_rule(value.selector, value.properties, value.transitions, value.location);
						return true;
					}
					else
					{
						nodes.push_back(value);
						return true;
					}
				};

				if (!etl::visit(visitor, item))
				{
					return false;
				}
			}

			return true;
		}
	}// namespace Markup


	trinex_implement_ui_element(Document) {}

	Document::Document()
	{
		document(this);
		m_bindings = trx_new Refl::NativeType<void>();
	}

	Document::~Document()
	{
		trx_delete_inline(m_bindings);
	}

	bool Document::load(StringView source, const Path& path)
	{
		Vector<Markup::Node> roots;
		Markup::StyleMap styles;

		if (!Markup::load_items(roots, styles, source, path, 0))
		{
			return false;
		}

		clear();
		m_elements.clear();
		m_style_sheet = etl::move(styles);

		for (const Markup::Node& root : roots)
		{
			if (!create_elements(this, this, root))
			{
				clear();
				m_elements.clear();
				return false;
			}
		}

		return true;
	}

	bool Document::load(const Path& path)
	{
		FileReader reader(path);

		if (!reader.is_open())
		{
			trinex_error(Log::Editor, "Failed to open UI document '%s'", path.c_str());
			return false;
		}

		return load(reader.read_string(), path);
	}

	Document& Document::open()
	{
		m_open = true;
		return *this;
	}

	Document& Document::close()
	{
		m_open = false;
		return *this;
	}

	Document& Document::register_element(Name id, Element* element)
	{
		if (id.is_valid() && element)
		{
			m_elements[id] = element;
		}

		return *this;
	}

	bool Document::is_open() const
	{
		return m_open;
	}

	bool Document::is_closed() const
	{
		return !m_open;
	}

	Element* Document::find_element(Name id) const
	{
		auto it = m_elements.find(id);
		return it == m_elements.end() ? nullptr : it->second;
	}
}// namespace Trinex::UI
