#include "../internal.hpp"
#include <Core/etl/string.hpp>
#include <Core/etl/utility.hpp>
#include <Core/etl/variant.hpp>
#include <Core/etl/vector.hpp>
#include <UI/Elements/document.hpp>
#include <UI/element_registry.hpp>
#include <peglib.h>

namespace Trinex::UI
{
	namespace Markup
	{
		inline constexpr const char* grammar = R"(
Document <- Node* ~EndOfFile

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
Binding      <- < '$' Identifier ('.' Identifier)* >

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

		struct Node {
			String type;
			Vector<Property> properties;
			Vector<Node> children;
			SourceLocation location;
		};

		using Member = Variant<Property, Node>;

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
						binding.emplace_back(Detail::any_ref<Identifier>(values[index]));
					}

					return ValueDesc{
					        .value    = Value{binding},
					        .location = Detail::location_of(values),
					};
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

				m_parser["Document"] = [](const peg::SemanticValues& values) {
					Vector<Node> nodes;
					nodes.reserve(values.size());

					for (const std::any& value : values)
					{
						nodes.emplace_back(Detail::any_ref<Node>(value));
					}

					return nodes;
				};
			}

		public:
			static Parser* instance()
			{
				static thread_local Parser parser;
				return &parser;
			}

			bool parse(Vector<Node>& nodes, StringView source)
			{
				if (source.empty())
				{
					return false;
				}

				Vector<Node> result;

				const bool success = m_parser.parse(source, result);

				if (!success)
				{
					return false;
				}

				nodes = etl::move(result);
				return true;
			}
		};

		static bool create_elements(Element* owner, const Node& node)
		{
			auto element = owner->attach(node.type);

			if (element == nullptr)
			{
				trinex_error(Log::Editor, "Failed to create element '%s' at %u:%u", node.type.c_str(), node.location.line,
				             node.location.column);
				return false;
			}

			auto type = element->type();

			for (auto& prop : node.properties)
			{
				if (!type->property(element, prop.name, prop.value))
				{
					trinex_error(Log::Editor, "Failed to assign property '%s' of element '%s' at %u:%u", prop.name.c_str(),
					             node.type.c_str(), prop.location.line, prop.location.column);
					return false;
				}
			}

			for (auto& child : node.children)
			{
				if (!create_elements(element, child))
				{
					return false;
				}
			}

			return true;
		}
	}// namespace Markup


	trinex_implement_ui_element(Document) {}

	Document* create_document(StringView source)
	{
		auto parser = Markup::Parser::instance();
		Vector<Markup::Node> roots;

		if (parser->parse(roots, source))
		{
			Document* document = trx_new Document();

			bool success = true;
			for (const Markup::Node& root : roots)
			{
				if (!create_elements(document, root))
				{
					success = false;
					break;
				}
			}

			if (success)
				return document;

			document->release();
			return nullptr;
		}

		return nullptr;
	}
}// namespace Trinex::UI
