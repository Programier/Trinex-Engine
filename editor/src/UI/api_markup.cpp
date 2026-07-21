#include <Core/etl/string.hpp>
#include <Core/etl/utility.hpp>
#include <Core/etl/variant.hpp>
#include <Core/etl/vector.hpp>
#include <peglib.h>

namespace Trinex::UI
{
	namespace Markup
	{
		inline constexpr const char* grammar = R"(
Document <- Node EndOfFile

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

Localization <- < '@' Path >
Binding      <- < '$' Path >
Path         <- Identifier ('.' Identifier)*

List <- '[' (Value (',' Value)*)? ']'

String     <- < '"' (Escape / StringChar)* '"' > { no_whitespace }
Escape     <- '\\' ['"\\nrt]
StringChar <- !['"\\] .

Float   <- < [+-]? [0-9]+ '.' [0-9]+ ([eE] [+-]? [0-9]+)? >
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

		struct SourceLocation {
			u32 line   = 0;
			u32 column = 0;
		};

		struct LocalizationKey {
			String value;
		};

		struct BindingPath {
			String value;
		};

		struct Identifier {
			String value;
		};

		struct Null {
		};

		struct Value;
		struct Property;
		struct Node;

		using Scalar    = Variant<Null, bool, i64, f64, String, LocalizationKey, BindingPath, Identifier>;
		using Container = Vector<struct Value>;

		struct Value {
			Variant<Scalar, Container> value;
			SourceLocation location;
		};

		struct Property {
			String name;
			Value value;
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

					if (index + 1 > token.size())
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
		}// namespace Detail

		class Parser
		{
		private:
			peg::parser m_parser;

		private:
			Parser() : m_parser(grammar) {}

			void configure_logger() {}

			void configure_actions()
			{
				configure_leaf_actions();
				configure_value_actions();
				configure_tree_actions();
			}

			void configure_leaf_actions()
			{
				m_parser["Identifier"] = [](const peg::SemanticValues& values) {
					return Identifier{.value = String(values.token())};
				};

				m_parser["Path"] = [](const peg::SemanticValues& values) {
					String path;

					for (std::size_t index = 0; index < values.size(); ++index)
					{
						const auto& identifier = Detail::any_ref<Identifier>(values[index]);

						if (!path.empty())
						{
							path += '.';
						}

						path += identifier.value;
					}

					return path;
				};

				m_parser["IdentifierValue"] = [](const peg::SemanticValues& values) {
					const auto& identifier = Detail::any_ref<Identifier>(values[0]);

					return Value{
					        .value    = Scalar{identifier},
					        .location = Detail::location_of(values),
					};
				};

				m_parser["Localization"] = [](const peg::SemanticValues& values) {
					StringView token = values.token();

					if (!token.empty() && token.front() == '@')
					{
						token.remove_prefix(1);
					}

					return Value{
					        .value    = Scalar{LocalizationKey{.value = String(token)}},
					        .location = Detail::location_of(values),
					};
				};

				m_parser["Binding"] = [](const peg::SemanticValues& values) {
					StringView token = values.token();

					if (!token.empty() && token.front() == '$')
					{
						token.remove_prefix(1);
					}

					return Value{
					        .value    = Scalar{BindingPath{.value = String(token)}},
					        .location = Detail::location_of(values),
					};
				};

				m_parser["String"] = [](const peg::SemanticValues& values) {
					return Value{
					        .value    = Scalar{Detail::unescape_string(values.token())},
					        .location = Detail::location_of(values),
					};
				};

				m_parser["Integer"] = [](const peg::SemanticValues& values) {
					return Value{
					        .value    = Scalar{values.token_to_number<i64>()},
					        .location = Detail::location_of(values),
					};
				};

				m_parser["Float"] = [](const peg::SemanticValues& values) {
					return Value{
					        .value    = Scalar{values.token_to_number<f64>()},
					        .location = Detail::location_of(values),
					};
				};

				m_parser["Boolean"] = [](const peg::SemanticValues& values) {
					return Value{
					        .value    = Scalar{values.token() == "true"},
					        .location = Detail::location_of(values),
					};
				};

				m_parser["Null"] = [](const peg::SemanticValues& values) {
					return Value{.value = Scalar{Null{}}, .location = Detail::location_of(values)};
				};
			}

			void configure_value_actions()
			{
				m_parser["List"] = [](const peg::SemanticValues& values) {
					Container list;
					list.reserve(values.size());

					for (const std::any& item : values)
					{
						list.emplace_back(Detail::any_ref<Value>(item));
					}

					return Value{.value = std::move(list), .location = Detail::location_of(values)};
				};

				m_parser["Value"]  = [](const peg::SemanticValues& values) { return Detail::any_ref<Value>(values[0]); };
				m_parser["Member"] = [](const peg::SemanticValues& values) -> std::any { return values[0]; };
			}

			void configure_tree_actions()
			{
				m_parser["Property"] = [](const peg::SemanticValues& values) {
					const auto& identifier = Detail::any_ref<Identifier>(values[0]);

					return Property{
					        .name     = identifier.value,
					        .value    = Detail::any_ref<Value>(values[1]),
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
					        .type       = identifier.value,
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

				m_parser["Document"] = [](const peg::SemanticValues& values) { return Detail::any_ref<Node>(values[0]); };
			}

		public:
			static Parser* instance()
			{
				static thread_local Parser parser;
				return &parser;
			}

			bool parse(Node& node, StringView source)
			{
				if (source.empty())
				{
					return false;
				}

				Node result;

				const bool success = m_parser.parse(source, result);

				if (!success)
				{
					return false;
				}

				node = etl::move(result);
				return true;
			}
		};

		static void dump_value(const Value& value);

		static void print_indent(u32 indent)
		{
			for (u32 i = 0; i < indent; ++i)
			{
				std::printf("  ");
			}
		}

		static void dump_value(const Markup::Value& value)
		{
			auto visitor = [](const auto& data) {
				using Type = std::remove_cvref_t<decltype(data)>;

				if constexpr (std::is_same_v<Type, Markup::Scalar>)
				{
					auto visitor = [](const auto& scalar) {
						using ScalarType = std::remove_cvref_t<decltype(scalar)>;

						if constexpr (std::is_same_v<ScalarType, Markup::Null>)
						{
							std::printf("null");
						}
						else if constexpr (std::is_same_v<ScalarType, bool>)
						{
							std::printf("%s", scalar ? "true" : "false");
						}
						else if constexpr (std::is_same_v<ScalarType, i64>)
						{
							std::printf("%lld", static_cast<long long>(scalar));
						}
						else if constexpr (std::is_same_v<ScalarType, f64>)
						{
							std::printf("%g", scalar);
						}
						else if constexpr (std::is_same_v<ScalarType, String>)
						{
							std::printf("\"%s\"", scalar.c_str());
						}
						else if constexpr (std::is_same_v<ScalarType, Markup::LocalizationKey>)
						{
							std::printf("@%s", scalar.value.c_str());
						}
						else if constexpr (std::is_same_v<ScalarType, Markup::BindingPath>)
						{
							std::printf("$%s", scalar.value.c_str());
						}
						else if constexpr (std::is_same_v<ScalarType, Markup::Identifier>)
						{
							std::printf("%s", scalar.value.c_str());
						}
					};
					etl::visit(visitor, data);
				}
				else if constexpr (std::is_same_v<Type, Container>)
				{
					std::printf("[");

					for (usize index = 0; index < data.size(); ++index)
					{
						if (index != 0)
						{
							std::printf(", ");
						}

						dump_value(data[index]);
					}

					std::printf("]");
				}
			};

			etl::visit(visitor, value.value);
		}

		static void dump(const Node& node, u32 indent = 0)
		{
			print_indent(indent);
			std::printf("%s {\n", node.type.c_str());

			for (const Markup::Property& property : node.properties)
			{
				print_indent(indent + 1);

				std::printf("%s: ", property.name.c_str());
				dump_value(property.value);
				std::printf("\n");
			}

			for (const Markup::Node& child : node.children)
			{
				dump(child, indent + 1);
			}

			print_indent(indent);
			std::printf("}\n");
		}
	}// namespace Markup


	trinex_on_pre_init()
	{
		auto parser = Markup::Parser::instance();

		Markup::Node node;

		bool status = parser->parse(node, R"(
			Window{
				Button {
					text: @Common.Save
					enabled: $Project.CanSave
				}
			}
		)");

		printf("\n\n\n\nSTATUS: %d\n", status);
		Markup::dump(node);
		exit(0);
	}
}// namespace Trinex::UI
