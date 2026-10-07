#include <algorithm>
#include <cctype>
#include <model.hpp>
#include <parser.hpp>
#include <set>
#include <string>
#include <tokenizer.hpp>
#include <utility>
#include <vector>

namespace Reflector
{
	namespace
	{
		struct Cursor {
			std::string_view source;
			const std::vector<Token>* tokens = nullptr;
			std::size_t index                = 0;
		};

		void add_diagnostic(TranslationUnit& unit, DiagnosticSeverity severity, const Annotation& annotation, std::string message)
		{
			Diagnostic diagnostic;
			diagnostic.severity = severity;
			diagnostic.message  = std::move(message);
			diagnostic.line     = annotation.line;
			diagnostic.column   = annotation.column;
			unit.diagnostics.push_back(std::move(diagnostic));
		}

		bool is_eof(const Token& token)
		{
			return token.type == TokenType::EndOfFile;
		}

		bool is_identifier(const Token& token, std::string_view value = {})
		{
			return token.type == TokenType::Identifier && (value.empty() || token.value == value);
		}

		bool is_symbol(const Token& token, std::string_view value)
		{
			return token.type == TokenType::Symbol && token.value == value;
		}

		bool is_trivia(const Token& token)
		{
			return token.type == TokenType::Comment || token.type == TokenType::Preprocessor;
		}

		const Token& token_at(const Cursor& cursor, std::size_t index)
		{
			return (*cursor.tokens)[std::min(index, cursor.tokens->size() - 1)];
		}

		const Token& current(const Cursor& cursor)
		{
			return token_at(cursor, cursor.index);
		}

		void skip_trivia(Cursor& cursor)
		{
			while (is_trivia(current(cursor)))
			{
				++cursor.index;
			}
		}

		std::string trim(std::string_view value)
		{
			while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())))
			{
				value.remove_prefix(1);
			}
			while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())))
			{
				value.remove_suffix(1);
			}
			return std::string(value);
		}

		std::string join_scope(const std::vector<std::string>& scope)
		{
			std::string result;
			for (const auto& part : scope)
			{
				if (!result.empty())
				{
					result += "::";
				}
				result += part;
			}
			return result;
		}

		std::string qualify(const std::vector<std::string>& scope, std::string_view name)
		{
			auto prefix = join_scope(scope);
			if (prefix.empty())
			{
				return std::string(name);
			}
			return prefix + "::" + std::string(name);
		}

		std::string raw_between(std::string_view source, const Token& begin, const Token& end)
		{
			if (end.offset <= begin.offset)
			{
				return {};
			}
			return trim(source.substr(begin.offset, end.offset - begin.offset));
		}

		std::string raw_range(std::string_view source, const Token& begin, const Token& end)
		{
			if (end.offset < begin.offset)
			{
				return {};
			}
			return trim(source.substr(begin.offset, end.offset + end.length - begin.offset));
		}

		bool contains_word(std::string_view value, std::string_view word)
		{
			auto offset = value.find(word);
			while (offset != std::string_view::npos)
			{
				const auto before =
				        offset == 0 || !(std::isalnum(static_cast<unsigned char>(value[offset - 1])) || value[offset - 1] == '_');
				const auto after = offset + word.size() >= value.size() ||
				                   !(std::isalnum(static_cast<unsigned char>(value[offset + word.size()])) ||
				                     value[offset + word.size()] == '_');
				if (before && after)
				{
					return true;
				}
				offset = value.find(word, offset + word.size());
			}
			return false;
		}

		bool has_suffix(std::string_view value, std::string_view suffix)
		{
			return value.size() >= suffix.size() && value.substr(value.size() - suffix.size()) == suffix;
		}

		bool is_type_prefix(const Token& token)
		{
			static const std::set<std::string> prefixes = {"static",       "virtual",      "inline",    "constexpr", "consteval",
			                                               "explicit",     "friend",       "mutable",   "volatile",  "extern",
			                                               "thread_local", "FORCE_INLINE", "DEPRECATED"};
			return token.type == TokenType::Identifier && prefixes.find(token.value) != prefixes.end();
		}

		bool is_export_macro(const Token& token)
		{
			return token.type == TokenType::Identifier &&
			       (has_suffix(token.value, "_API") || has_suffix(token.value, "_EXPORT") || has_suffix(token.value, "_IMPORT"));
		}

		bool is_engine_macro(const Token& token)
		{
			if (token.type != TokenType::Identifier)
			{
				return false;
			}
			return is_export_macro(token) || has_suffix(token.value, "_API") || has_suffix(token.value, "_INLINE") ||
			       has_suffix(token.value, "_MACRO") || token.value == "GENERATED_BODY" ||
			       token.value == "GENERATED_CLASS_BODY" || token.value == "FORCE_INLINE" || token.value == "FORCEINLINE";
		}

		bool is_template_argument_list(const std::vector<Token>& tokens, std::size_t index)
		{
			return index > 0 && tokens[index - 1].type == TokenType::Identifier && is_symbol(tokens[index], "<");
		}

		std::size_t matching_token(const std::vector<Token>& tokens, std::size_t open_index);

		std::string consume_template_prefix(Cursor& cursor)
		{
			skip_trivia(cursor);
			if (!is_identifier(current(cursor), "template"))
			{
				return {};
			}

			const auto begin = cursor.index++;
			skip_trivia(cursor);
			if (!is_symbol(current(cursor), "<"))
			{
				cursor.index = begin;
				return {};
			}

			const auto close = matching_token(*cursor.tokens, cursor.index);
			if (close == cursor.index)
			{
				cursor.index = begin;
				return {};
			}

			const auto prefix = raw_range(cursor.source, token_at(cursor, begin), token_at(cursor, close));
			cursor.index      = close + 1;
			return prefix;
		}

		std::string append_raw(std::string current, std::string_view value)
		{
			if (value.empty())
			{
				return current;
			}
			if (!current.empty())
			{
				current += ' ';
			}
			current += value;
			return current;
		}

		struct DeclarationPrefix {
			std::string attributes;
			std::string engine_macros;
		};

		DeclarationPrefix consume_declaration_prefix(Cursor& cursor)
		{
			DeclarationPrefix prefix;

			for (;;)
			{
				skip_trivia(cursor);

				if (is_symbol(current(cursor), "[") && is_symbol(token_at(cursor, cursor.index + 1), "["))
				{
					const auto begin = cursor.index;
					++cursor.index;
					const auto close = matching_token(*cursor.tokens, cursor.index);
					if (close == cursor.index)
					{
						cursor.index = begin;
						break;
					}
					const auto end =
					        close + 1 < cursor.tokens->size() && is_symbol(token_at(cursor, close + 1), "]") ? close + 1 : close;
					prefix.attributes = append_raw(prefix.attributes,
					                               raw_range(cursor.source, token_at(cursor, begin), token_at(cursor, end)));
					cursor.index      = end + 1;
					continue;
				}

				if (is_identifier(current(cursor), "alignas") && is_symbol(token_at(cursor, cursor.index + 1), "("))
				{
					const auto begin = cursor.index;
					cursor.index += 1;
					const auto close = matching_token(*cursor.tokens, cursor.index);
					if (close == cursor.index)
					{
						cursor.index = begin;
						break;
					}
					prefix.attributes = append_raw(prefix.attributes,
					                               raw_range(cursor.source, token_at(cursor, begin), token_at(cursor, close)));
					cursor.index      = close + 1;
					continue;
				}

				if (is_engine_macro(current(cursor)))
				{
					const auto begin = cursor.index++;
					if (is_symbol(current(cursor), "("))
					{
						const auto close = matching_token(*cursor.tokens, cursor.index);
						if (close != cursor.index)
						{
							prefix.engine_macros =
							        append_raw(prefix.engine_macros,
							                   raw_range(cursor.source, token_at(cursor, begin), token_at(cursor, close)));
							cursor.index = close + 1;
							continue;
						}
					}
					prefix.engine_macros = append_raw(prefix.engine_macros, token_at(cursor, begin).value);
					continue;
				}

				break;
			}

			return prefix;
		}

		std::size_t matching_token(const std::vector<Token>& tokens, std::size_t open_index)
		{
			const auto& open = tokens[open_index];
			std::string close;
			if (open.value == "(")
			{
				close = ")";
			}
			else if (open.value == "{")
			{
				close = "}";
			}
			else if (open.value == "[")
			{
				close = "]";
			}
			else if (open.value == "<")
			{
				close = ">";
			}
			else
			{
				return open_index;
			}

			std::size_t depth = 0;
			for (std::size_t i = open_index; i < tokens.size(); ++i)
			{
				if (tokens[i].value == open.value)
				{
					++depth;
				}
				else if (tokens[i].value == close)
				{
					if (--depth == 0)
					{
						return i;
					}
				}
			}
			return open_index;
		}

		std::size_t skip_balanced(const std::vector<Token>& tokens, std::size_t index)
		{
			if (is_symbol(tokens[index], "(") || is_symbol(tokens[index], "{") || is_symbol(tokens[index], "[") ||
			    is_template_argument_list(tokens, index))
			{
				return matching_token(tokens, index) + 1;
			}
			return index + 1;
		}

		std::vector<std::pair<std::size_t, std::size_t>> split_token_ranges(const std::vector<Token>& tokens, std::size_t begin,
		                                                                    std::size_t end, std::string_view separator)
		{
			std::vector<std::pair<std::size_t, std::size_t>> ranges;
			std::size_t range_begin = begin;
			for (std::size_t i = begin; i < end;)
			{
				if (is_symbol(tokens[i], "(") || is_symbol(tokens[i], "{") || is_symbol(tokens[i], "[") ||
				    is_template_argument_list(tokens, i))
				{
					i = skip_balanced(tokens, i);
					continue;
				}
				if (tokens[i].value == separator)
				{
					if (range_begin < i)
					{
						ranges.emplace_back(range_begin, i);
					}
					range_begin = i + 1;
				}
				++i;
			}
			if (range_begin < end)
			{
				ranges.emplace_back(range_begin, end);
			}
			return ranges;
		}

		std::vector<std::string> split_qualified_name(std::string_view qualified_name)
		{
			std::vector<std::string> parts;
			std::size_t begin = 0;
			while (begin < qualified_name.size())
			{
				const auto separator = qualified_name.find("::", begin);
				if (separator == std::string_view::npos)
				{
					parts.push_back(std::string(qualified_name.substr(begin)));
					break;
				}
				parts.push_back(std::string(qualified_name.substr(begin, separator - begin)));
				begin = separator + 2;
			}
			return parts;
		}

		TypeInfo parse_type_info(std::string_view raw_type)
		{
			TypeInfo info;
			info.raw = trim(raw_type);
			if (info.raw.empty())
			{
				return info;
			}

			Tokenizer tokenizer;
			auto tokens       = tokenizer.tokenize(info.raw);
			std::size_t begin = 0;
			std::size_t end   = tokens.size() - 1;

			while (begin < end &&
			       (tokens[begin].value == "static" || tokens[begin].value == "mutable" || tokens[begin].value == "constexpr" ||
			        tokens[begin].value == "inline" || tokens[begin].value == "extern" || tokens[begin].value == "thread_local"))
			{
				++begin;
			}

			while (begin < end && (tokens[begin].value == "const" || tokens[begin].value == "volatile"))
			{
				if (tokens[begin].value == "const")
					info.flags |= TypeFlag_Const;
				else
					info.flags |= TypeFlag_Volatile;
				++begin;
			}

			while (end > begin &&
			       (tokens[end - 1].value == "*" || tokens[end - 1].value == "&" || tokens[end - 1].value == "&&" ||
			        tokens[end - 1].value == "const" || tokens[end - 1].value == "volatile"))
			{
				const auto& token = tokens[end - 1];
				if (token.value == "*")
				{
					info.flags |= TypeFlag_Pointer;
					++info.pointer_depth;
				}
				else if (token.value == "&")
				{
					info.flags |= TypeFlag_Reference;
				}
				else if (token.value == "&&")
				{
					info.flags |= TypeFlag_Reference | TypeFlag_RValueRef;
				}
				else if (token.value == "const")
				{
					info.flags |= TypeFlag_Const;
				}
				else if (token.value == "volatile")
				{
					info.flags |= TypeFlag_Volatile;
				}
				--end;
			}

			if (begin >= end)
			{
				return info;
			}

			std::size_t template_begin = end;
			for (std::size_t i = begin; i < end; ++i)
			{
				if (is_template_argument_list(tokens, i))
				{
					template_begin = i;
					break;
				}
			}

			const auto name_end = template_begin == end ? end : template_begin;
			info.qualified_name = raw_between(info.raw, tokens[begin], tokens[name_end]);
			auto parts          = split_qualified_name(info.qualified_name);
			if (!parts.empty())
			{
				info.name = parts.back();
				parts.pop_back();
				info.namespaces = std::move(parts);
			}

			if (template_begin < end)
			{
				const auto template_end = matching_token(tokens, template_begin);
				if (template_end > template_begin)
				{
					for (const auto& range : split_token_ranges(tokens, template_begin + 1, template_end, ","))
					{
						const auto argument = raw_between(info.raw, tokens[range.first], tokens[range.second]);
						info.template_arguments.push_back(parse_type_info(argument));
					}
				}
			}

			return info;
		}

		std::string type_info_flags_to_string(const TypeInfo& info)
		{
			std::string result;
			if (info.flags & TypeFlag_Const)
				result = append_raw(result, "const");
			if (info.flags & TypeFlag_Volatile)
				result = append_raw(result, "volatile");
			if (info.flags & TypeFlag_Pointer)
				result = append_raw(result, "pointer");
			if (info.flags & TypeFlag_Reference)
				result = append_raw(result, "reference");
			if (info.flags & TypeFlag_RValueRef)
				result = append_raw(result, "rvalue_ref");
			return result;
		}

		std::vector<Annotation::Argument> parse_annotation_metadata(std::string_view arguments)
		{
			Tokenizer tokenizer;
			auto tokens = tokenizer.tokenize(arguments);
			std::vector<Annotation::Argument> output;

			for (const auto& range : split_token_ranges(tokens, 0, tokens.size() - 1, ","))
			{
				std::size_t equals = range.second;
				for (std::size_t i = range.first; i < range.second; ++i)
				{
					if (tokens[i].value == "=")
					{
						equals = i;
						break;
					}
				}

				Annotation::Argument argument;
				if (equals == range.second)
				{
					argument.value = raw_range(arguments, tokens[range.first], tokens[range.second - 1]);
				}
				else
				{
					argument.name  = raw_between(arguments, tokens[range.first], tokens[equals]);
					argument.value = raw_between(arguments, tokens[equals + 1], tokens[range.second]);
				}
				output.push_back(std::move(argument));
			}
			return output;
		}

		bool parse_annotation(Cursor& cursor, Annotation& annotation)
		{
			skip_trivia(cursor);
			const auto& token = current(cursor);
			if (!is_identifier(token) || token.value.find("trinex_") != 0)
			{
				return false;
			}

			const auto annotation_begin = cursor.index++;
			skip_trivia(cursor);
			if (!is_symbol(current(cursor), "("))
			{
				cursor.index = annotation_begin + 1;
				return false;
			}

			const auto open  = cursor.index;
			const auto close = matching_token(*cursor.tokens, open);
			if (close == open)
			{
				return false;
			}

			annotation           = {};
			annotation.name      = token.value;
			annotation.line      = token.line;
			annotation.column    = token.column;
			annotation.arguments = close == open + 1
			                               ? std::string()
			                               : raw_between(cursor.source, token_at(cursor, open + 1), token_at(cursor, close));
			annotation.metadata  = parse_annotation_metadata(annotation.arguments);

			cursor.index = close + 1;
			skip_trivia(cursor);
			if (is_symbol(current(cursor), ";"))
			{
				++cursor.index;
			}
			return true;
		}

		std::size_t find_statement_end(const std::vector<Token>& tokens, std::size_t begin)
		{
			for (std::size_t i = begin; i < tokens.size();)
			{
				if (is_symbol(tokens[i], "(") || is_symbol(tokens[i], "[") || is_template_argument_list(tokens, i))
				{
					i = skip_balanced(tokens, i);
					continue;
				}
				if (is_symbol(tokens[i], "{"))
				{
					return i;
				}
				if (is_symbol(tokens[i], ";") || is_eof(tokens[i]))
				{
					return i;
				}
				++i;
			}
			return tokens.size() - 1;
		}

		std::size_t find_top_level_equal(const std::vector<Token>& tokens, std::size_t begin, std::size_t end)
		{
			for (std::size_t i = begin; i < end;)
			{
				if (is_symbol(tokens[i], "(") || is_symbol(tokens[i], "{") || is_symbol(tokens[i], "[") ||
				    is_template_argument_list(tokens, i))
				{
					i = skip_balanced(tokens, i);
					continue;
				}
				if (is_symbol(tokens[i], "="))
				{
					return i;
				}
				++i;
			}
			return end;
		}

		std::size_t find_parameter_list_open(const std::vector<Token>& tokens, std::size_t begin, std::size_t end)
		{
			for (std::size_t i = begin; i < end; ++i)
			{
				if (is_symbol(tokens[i], "("))
				{
					const auto close = matching_token(tokens, i);
					const auto is_declarator_group =
					        i > begin && (is_symbol(tokens[i - 1], "*") || is_symbol(tokens[i - 1], "&") ||
					                      is_symbol(tokens[i - 1], "&&") || is_symbol(tokens[i - 1], "::"));
					if (close < end && !is_declarator_group)
					{
						return i;
					}
				}
			}
			return end;
		}

		std::string strip_type_prefixes(std::string type)
		{
			static const std::set<std::string> prefixes = {"static",    "virtual",      "inline",      "constexpr",
			                                               "consteval", "explicit",     "friend",      "mutable",
			                                               "extern",    "thread_local", "FORCE_INLINE"};
			for (;;)
			{
				type             = trim(type);
				const auto space = type.find_first_of(" \t\r\n");
				const auto word  = space == std::string::npos ? type : type.substr(0, space);
				if (prefixes.find(word) == prefixes.end())
				{
					return type;
				}
				type = space == std::string::npos ? std::string() : type.substr(space + 1);
			}
		}

		std::size_t find_decl_name(const std::vector<Token>& tokens, std::size_t begin, std::size_t end)
		{
			std::size_t best = end;
			for (std::size_t i = begin; i < end; ++i)
			{
				if (tokens[i].type != TokenType::Identifier)
				{
					continue;
				}
				if (i + 1 < end && is_symbol(tokens[i + 1], "::"))
				{
					continue;
				}
				if (is_type_prefix(tokens[i]) || is_export_macro(tokens[i]) || tokens[i].value == "const")
				{
					continue;
				}
				best = i;
			}
			return best;
		}

		std::size_t find_property_name(const std::vector<Token>& tokens, std::size_t begin, std::size_t end)
		{
			std::size_t best = end;
			for (std::size_t i = begin; i < end;)
			{
				if (is_symbol(tokens[i], "("))
				{
					const auto close = matching_token(tokens, i);
					if (close + 1 < end && is_symbol(tokens[close + 1], "("))
					{
						for (std::size_t nested = i + 1; nested < close; ++nested)
						{
							if (tokens[nested].type == TokenType::Identifier && !is_type_prefix(tokens[nested]))
							{
								best = nested;
							}
						}
					}
					i = close + 1;
					continue;
				}
				if (is_symbol(tokens[i], "[") || is_template_argument_list(tokens, i) || is_symbol(tokens[i], "{"))
				{
					i = skip_balanced(tokens, i);
					continue;
				}
				if (tokens[i].type == TokenType::Identifier && !is_type_prefix(tokens[i]) && !is_export_macro(tokens[i]) &&
				    tokens[i].value != "const")
				{
					best = i;
				}
				++i;
			}
			return best;
		}

		std::string declaration_without_token(std::string_view source, const Token& begin, const Token& end, const Token& removed)
		{
			auto before = trim(source.substr(begin.offset, removed.offset - begin.offset));
			auto after  = trim(source.substr(removed.offset + removed.length, end.offset - removed.offset - removed.length));
			if (before.empty())
			{
				return after;
			}
			if (after.empty())
			{
				return before;
			}
			return before + after;
		}

		void fill_property_metadata(Property& property)
		{
			if (contains_word(property.declaration, "static"))
				property.flags |= PropertyFlag_Static;
			if (contains_word(property.type, "const"))
				property.flags |= PropertyFlag_Const;
			if (contains_word(property.declaration, "constexpr"))
				property.flags |= PropertyFlag_Constexpr;
			if (contains_word(property.declaration, "mutable"))
				property.flags |= PropertyFlag_Mutable;
			if (property.type.find('*') != std::string::npos || property.declaration.find('*') != std::string::npos)
				property.flags |= PropertyFlag_Pointer;
			if (property.type.find('&') != std::string::npos || property.declaration.find('&') != std::string::npos)
				property.flags |= PropertyFlag_Reference;
		}

		void fill_function_metadata(Function& function)
		{
			if (contains_word(function.declaration, "static"))
				function.flags |= FunctionFlag_Static;
			if (contains_word(function.declaration, "virtual") || contains_word(function.qualifiers, "override"))
				function.flags |= FunctionFlag_Virtual;
			if (contains_word(function.qualifiers, "const"))
				function.flags |= FunctionFlag_Const;
			if (contains_word(function.declaration, "constexpr"))
				function.flags |= FunctionFlag_Constexpr;
			if (contains_word(function.declaration, "inline") || contains_word(function.declaration, "FORCE_INLINE"))
				function.flags |= FunctionFlag_Inline;
			if (contains_word(function.qualifiers, "noexcept"))
				function.flags |= FunctionFlag_Noexcept;
			if (contains_word(function.qualifiers, "override"))
				function.flags |= FunctionFlag_Override;
			if (contains_word(function.qualifiers, "final"))
				function.flags |= FunctionFlag_Final;
			if (function.qualifiers.find("= 0") != std::string::npos || function.qualifiers.find("=0") != std::string::npos)
				function.flags |= FunctionFlag_PureVirtual;
		}

		void validate_property(TranslationUnit& unit, const Property& property)
		{
			if (property.name.empty())
			{
				add_diagnostic(unit, DiagnosticSeverity::Error, property.annotation, "reflected property has no parsed name");
			}
			if (property.type.empty())
			{
				add_diagnostic(unit, DiagnosticSeverity::Error, property.annotation, "reflected property has no parsed type");
			}
		}

		void validate_function(TranslationUnit& unit, const Function& function)
		{
			if (!function.template_prefix.empty())
			{
				add_diagnostic(unit, DiagnosticSeverity::Error, function.annotation,
				               "reflected template functions are not supported: " + function.full_name);
			}
			if (function.name.empty())
			{
				add_diagnostic(unit, DiagnosticSeverity::Error, function.annotation, "reflected function has no parsed name");
			}
			if (function.return_type.empty() && !(function.flags & FunctionFlag_Constructor) &&
			    !(function.flags & FunctionFlag_Destructor) && !(function.flags & FunctionFlag_Operator))
			{
				add_diagnostic(unit, DiagnosticSeverity::Error, function.annotation,
				               "reflected function has no parsed return type");
			}
		}

		void validate_type(TranslationUnit& unit, const Type& type)
		{
			if (!type.template_prefix.empty())
			{
				add_diagnostic(unit, DiagnosticSeverity::Error, type.annotation,
				               "reflected template classes are not supported: " + type.full_name);
			}
			if (type.name.empty())
			{
				add_diagnostic(unit, DiagnosticSeverity::Error, type.annotation, "reflected type has no parsed name");
			}
			for (const auto& property : type.properties)
			{
				validate_property(unit, property);
			}
			for (const auto& function : type.functions)
			{
				validate_function(unit, function);
			}
			for (const auto& nested_type : type.nested_types)
			{
				validate_type(unit, nested_type);
			}
		}

		void parse_parameter(const std::vector<Token>& tokens, std::string_view source, std::size_t begin, std::size_t end,
		                     Function::Parameter& parameter)
		{
			while (begin < end && is_identifier(tokens[begin]) && tokens[begin].value.find("trinex_") == 0)
			{
				Annotation ignored;
				Cursor cursor{source, &tokens, begin};
				if (!parse_annotation(cursor, ignored))
				{
					break;
				}
				begin = cursor.index;
			}

			parameter.declaration = raw_between(source, tokens[begin], tokens[end]);
			const auto equals     = find_top_level_equal(tokens, begin, end);
			if (equals != end)
			{
				parameter.default_value = raw_between(source, tokens[equals + 1], tokens[end]);
				end                     = equals;
			}
			const auto name_index = find_property_name(tokens, begin, end);
			if (name_index != end)
			{
				parameter.name = tokens[name_index].value;
				parameter.type = declaration_without_token(source, tokens[begin], tokens[end], tokens[name_index]);
			}
			else
			{
				parameter.type = parameter.declaration;
			}
			parameter.type      = strip_type_prefixes(parameter.type);
			parameter.type_info = parse_type_info(parameter.type);
		}

		std::vector<Function::Parameter> parse_parameters(const std::vector<Token>& tokens, std::string_view source,
		                                                  std::size_t begin, std::size_t end)
		{
			std::vector<Function::Parameter> parameters;
			if (begin >= end || (end == begin + 1 && tokens[begin].value == "void"))
			{
				return parameters;
			}
			for (const auto& range : split_token_ranges(tokens, begin, end, ","))
			{
				Function::Parameter parameter;
				parse_parameter(tokens, source, range.first, range.second, parameter);
				parameters.push_back(std::move(parameter));
			}
			return parameters;
		}

		Property parse_property(const std::vector<Token>& tokens, std::string_view source, Annotation annotation,
		                        std::size_t begin, std::size_t end, Access access, std::string owner = {},
		                        DeclarationPrefix prefix = {})
		{
			Property property;
			property.annotation    = std::move(annotation);
			property.access        = std::move(access);
			property.owner         = std::move(owner);
			property.attributes    = std::move(prefix.attributes);
			property.engine_macros = std::move(prefix.engine_macros);
			property.line          = property.annotation.line;
			property.declaration   = raw_between(source, tokens[begin], tokens[end]);

			const auto equals = find_top_level_equal(tokens, begin, end);
			if (equals != end)
			{
				property.default_value = raw_between(source, tokens[equals + 1], tokens[end]);
			}

			const auto name_end   = equals == end ? end : equals;
			const auto name_index = find_property_name(tokens, begin, name_end);
			if (name_index != name_end)
			{
				property.name = tokens[name_index].value;
				property.type = strip_type_prefixes(
				        declaration_without_token(source, tokens[begin], tokens[name_end], tokens[name_index]));
			}
			property.type_info = parse_type_info(property.type);
			property.full_name = property.owner.empty() ? property.name : property.owner + "::" + property.name;
			fill_property_metadata(property);
			return property;
		}

		std::string parse_operator_name(const std::vector<Token>& tokens, std::string_view source, std::size_t operator_index,
		                                std::size_t open)
		{
			if (operator_index + 1 >= open)
			{
				return "operator";
			}
			return raw_between(source, tokens[operator_index], tokens[open]);
		}

		Function parse_function(const std::vector<Token>& tokens, std::string_view source, Annotation annotation,
		                        std::size_t begin, std::size_t end, Access access, std::string owner = {},
		                        std::string template_prefix = {}, DeclarationPrefix prefix = {})
		{
			Function function;
			function.annotation      = std::move(annotation);
			function.access          = std::move(access);
			function.owner           = std::move(owner);
			function.template_prefix = std::move(template_prefix);
			function.attributes      = std::move(prefix.attributes);
			function.engine_macros   = std::move(prefix.engine_macros);
			function.line            = function.annotation.line;
			function.declaration     = raw_between(source, tokens[begin], tokens[end]);

			const auto open = find_parameter_list_open(tokens, begin, end);
			if (open != end)
			{
				const auto close = matching_token(tokens, open);
				auto name_index  = open > begin ? open - 1 : begin;
				if (open >= begin + 2 && is_symbol(tokens[open - 2], "~"))
				{
					function.name = "~";
					function.name += tokens[open - 1].value;
					function.flags |= FunctionFlag_Destructor;
					name_index = open - 2;
				}
				else
				{
					for (std::size_t i = begin; i < open; ++i)
					{
						if (is_identifier(tokens[i], "operator"))
						{
							function.name = parse_operator_name(tokens, source, i, open);
							function.flags |= FunctionFlag_Operator;
							name_index = i;
							break;
						}
					}
					if (function.name.empty())
					{
						function.name = tokens[name_index].value;
					}
				}
				function.return_type      = name_index > begin
				                                    ? strip_type_prefixes(raw_between(source, tokens[begin], tokens[name_index]))
				                                    : std::string();
				function.return_type_info = parse_type_info(function.return_type);
				if (function.return_type.empty() && !(function.flags & FunctionFlag_Destructor) &&
				    !(function.flags & FunctionFlag_Operator))
				{
					function.flags |= FunctionFlag_Constructor;
				}
				function.parameters = raw_between(source, tokens[open + 1], tokens[close]);
				function.qualifiers = close + 1 < end ? raw_between(source, tokens[close + 1], tokens[end]) : std::string();
				function.parsed_parameters = parse_parameters(tokens, source, open + 1, close);
			}
			function.full_name = function.owner.empty() ? function.name : function.owner + "::" + function.name;
			fill_function_metadata(function);
			return function;
		}

		Access parse_access(Access current_access, const Token& token, const Token& next)
		{
			if (next.value == ":")
			{
				if (token.value == "public")
					return Access::Public;
				if (token.value == "protected")
					return Access::Protected;
				if (token.value == "private")
					return Access::Private;
			}
			return current_access;
		}

		bool parse_type(Cursor& cursor, Annotation annotation, Type& type, const std::vector<std::string>& scope,
		                std::string template_prefix = {}, DeclarationPrefix prefix = {});

		void parse_members(Type& type, const std::vector<Token>& tokens, std::string_view source, std::size_t begin,
		                   std::size_t end)
		{
			Cursor cursor{source, &tokens, begin};
			auto access = type.kind == "struct" ? Access::Public : Access::Private;

			while (cursor.index < end && !is_eof(current(cursor)))
			{
				if (cursor.index + 1 < end)
				{
					access = parse_access(access, current(cursor), token_at(cursor, cursor.index + 1));
				}

				const auto template_prefix = consume_template_prefix(cursor);
				const auto prefix          = consume_declaration_prefix(cursor);

				Annotation annotation;
				if (!parse_annotation(cursor, annotation))
				{
					++cursor.index;
					continue;
				}
				auto declaration_prefix           = prefix;
				const auto post_annotation_prefix = consume_declaration_prefix(cursor);
				declaration_prefix.attributes     = append_raw(declaration_prefix.attributes, post_annotation_prefix.attributes);
				declaration_prefix.engine_macros =
				        append_raw(declaration_prefix.engine_macros, post_annotation_prefix.engine_macros);

				const auto declaration_begin = cursor.index;
				const auto statement_end     = find_statement_end(tokens, declaration_begin);
				auto declaration_end         = statement_end;
				if (is_symbol(token_at(cursor, statement_end), "{"))
				{
					declaration_end = statement_end;
					cursor.index    = matching_token(tokens, statement_end) + 1;
				}
				else
				{
					cursor.index = statement_end + 1;
				}

				if (annotation.name == "trinex_property")
				{
					auto property = parse_property(tokens, source, annotation, declaration_begin, declaration_end, access,
					                               type.full_name, declaration_prefix);
					type.properties.push_back(std::move(property));
				}
				else if (annotation.name == "trinex_function")
				{
					auto function = parse_function(tokens, source, annotation, declaration_begin, declaration_end, access,
					                               type.full_name, template_prefix, declaration_prefix);
					type.functions.push_back(std::move(function));
				}
				else if (annotation.name == "trinex_class" || annotation.name == "trinex_struct" ||
				         annotation.name == "trinex_enum")
				{
					Type nested_type;
					std::vector<std::string> nested_scope;
					nested_scope.push_back(type.full_name);
					Cursor nested_cursor{source, &tokens, declaration_begin};
					if (parse_type(nested_cursor, annotation, nested_type, nested_scope, template_prefix, declaration_prefix))
					{
						type.nested_types.push_back(std::move(nested_type));
					}
				}
			}
		}

		std::vector<std::string> parse_enum_values(const std::vector<Token>& tokens, std::string_view source, std::size_t begin,
		                                           std::size_t end)
		{
			std::vector<std::string> values;
			for (const auto& range : split_token_ranges(tokens, begin, end, ","))
			{
				values.push_back(raw_between(source, tokens[range.first], tokens[range.second]));
			}
			return values;
		}

		bool parse_type(Cursor& cursor, Annotation annotation, Type& type, const std::vector<std::string>& scope,
		                std::string template_prefix, DeclarationPrefix prefix)
		{
			skip_trivia(cursor);
			const auto local_prefix = consume_declaration_prefix(cursor);
			prefix.attributes       = append_raw(prefix.attributes, local_prefix.attributes);
			prefix.engine_macros    = append_raw(prefix.engine_macros, local_prefix.engine_macros);

			if (!is_identifier(current(cursor), "class") && !is_identifier(current(cursor), "struct") &&
			    !is_identifier(current(cursor), "enum"))
			{
				return false;
			}

			const auto declaration_begin = cursor.index;
			type.annotation              = std::move(annotation);
			type.template_prefix         = std::move(template_prefix);
			type.kind                    = current(cursor).value;
			++cursor.index;

			if (type.kind == "enum" && (current(cursor).value == "class" || current(cursor).value == "struct"))
			{
				++cursor.index;
			}

			while (is_engine_macro(current(cursor)) || current(cursor).value == "final")
			{
				if (is_engine_macro(current(cursor)))
				{
					prefix.engine_macros = append_raw(prefix.engine_macros, current(cursor).value);
				}
				++cursor.index;
			}

			type.attributes    = prefix.attributes;
			type.engine_macros = prefix.engine_macros;

			if (!is_identifier(current(cursor)))
			{
				return false;
			}

			type.name      = current(cursor).value;
			type.scope     = join_scope(scope);
			type.full_name = qualify(scope, type.name);
			type.line      = type.annotation.line;
			++cursor.index;

			std::size_t bases_begin = cursor.index;
			if (is_symbol(current(cursor), ":"))
			{
				bases_begin = ++cursor.index;
			}

			while (!is_symbol(current(cursor), "{") && !is_eof(current(cursor)))
			{
				++cursor.index;
			}

			if (is_eof(current(cursor)))
			{
				return false;
			}

			const auto body_begin = cursor.index;
			const auto body_end   = matching_token(*cursor.tokens, body_begin);
			if (body_end == body_begin)
			{
				return false;
			}

			if (bases_begin < body_begin && token_at(cursor, bases_begin - 1).value == ":")
			{
				type.bases = raw_between(cursor.source, token_at(cursor, bases_begin), token_at(cursor, body_begin));
			}

			type.declaration = raw_range(cursor.source, token_at(cursor, declaration_begin), token_at(cursor, body_end));
			if (type.kind == "enum")
			{
				type.enum_values = parse_enum_values(*cursor.tokens, cursor.source, body_begin + 1, body_end);
			}
			else
			{
				parse_members(type, *cursor.tokens, cursor.source, body_begin + 1, body_end);
			}

			cursor.index = body_end + 1;
			if (is_symbol(current(cursor), ";"))
			{
				++cursor.index;
			}
			return true;
		}

		bool parse_namespace(TranslationUnit& unit, const std::vector<Token>& tokens, std::string_view source, Cursor& cursor,
		                     const std::vector<std::string>& scope);

		void parse_range(TranslationUnit& unit, const std::vector<Token>& tokens, std::string_view source, std::size_t begin,
		                 std::size_t end, const std::vector<std::string>& scope)
		{
			Cursor cursor{source, &tokens, begin};

			while (cursor.index < end && !is_eof(current(cursor)))
			{
				skip_trivia(cursor);
				if (cursor.index >= end || is_eof(current(cursor)))
				{
					break;
				}

				if ((is_identifier(current(cursor), "namespace") || is_identifier(current(cursor), "inline")) &&
				    parse_namespace(unit, tokens, source, cursor, scope))
				{
					continue;
				}

				const auto template_prefix = consume_template_prefix(cursor);
				const auto prefix          = consume_declaration_prefix(cursor);

				Annotation annotation;
				if (!parse_annotation(cursor, annotation))
				{
					++cursor.index;
					continue;
				}
				auto declaration_prefix           = prefix;
				const auto post_annotation_prefix = consume_declaration_prefix(cursor);
				declaration_prefix.attributes     = append_raw(declaration_prefix.attributes, post_annotation_prefix.attributes);
				declaration_prefix.engine_macros =
				        append_raw(declaration_prefix.engine_macros, post_annotation_prefix.engine_macros);

				const auto declaration_begin = cursor.index;
				if (annotation.name == "trinex_class" || annotation.name == "trinex_struct" || annotation.name == "trinex_enum")
				{
					Type type;
					if (parse_type(cursor, annotation, type, scope, template_prefix, declaration_prefix))
					{
						validate_type(unit, type);
						unit.types.push_back(std::move(type));
					}
					else
					{
						add_diagnostic(unit, DiagnosticSeverity::Error, annotation, "failed to parse annotated type");
						cursor.index = std::max(cursor.index, declaration_begin + 1);
					}
					continue;
				}

				const auto statement_end = find_statement_end(tokens, declaration_begin);
				auto declaration_end     = statement_end;
				if (is_symbol(tokens[statement_end], "{"))
				{
					declaration_end = statement_end;
					cursor.index    = matching_token(tokens, statement_end) + 1;
				}
				else
				{
					cursor.index = statement_end + 1;
				}

				const auto owner = join_scope(scope);
				if (annotation.name == "trinex_property")
				{
					auto property = parse_property(tokens, source, annotation, declaration_begin, declaration_end, Access::Global,
					                               owner, declaration_prefix);
					validate_property(unit, property);
					unit.properties.push_back(std::move(property));
				}
				else if (annotation.name == "trinex_function")
				{
					auto function = parse_function(tokens, source, annotation, declaration_begin, declaration_end, Access::Global,
					                               owner, template_prefix, declaration_prefix);
					validate_function(unit, function);
					unit.functions.push_back(std::move(function));
				}
			}
		}

		bool parse_namespace(TranslationUnit& unit, const std::vector<Token>& tokens, std::string_view source, Cursor& cursor,
		                     const std::vector<std::string>& scope)
		{
			auto index = cursor.index;
			if (is_identifier(tokens[index], "inline"))
			{
				++index;
			}

			if (!is_identifier(tokens[index], "namespace"))
			{
				return false;
			}
			++index;

			std::vector<std::string> namespace_parts;
			while (!is_eof(tokens[index]) && !is_symbol(tokens[index], "{") && !is_symbol(tokens[index], ";") &&
			       !is_symbol(tokens[index], "="))
			{
				if (tokens[index].type == TokenType::Identifier)
				{
					namespace_parts.push_back(tokens[index].value);
				}
				++index;
			}

			if (is_symbol(tokens[index], "="))
			{
				while (!is_eof(tokens[index]) && !is_symbol(tokens[index], ";"))
				{
					++index;
				}
				cursor.index = is_symbol(tokens[index], ";") ? index + 1 : index;
				return true;
			}

			if (!is_symbol(tokens[index], "{"))
			{
				return false;
			}

			const auto body_begin = index;
			const auto body_end   = matching_token(tokens, body_begin);
			if (body_end == body_begin)
			{
				return false;
			}

			auto nested_scope = scope;
			for (auto& part : namespace_parts)
			{
				nested_scope.push_back(std::move(part));
			}

			parse_range(unit, tokens, source, body_begin + 1, body_end, nested_scope);
			cursor.index = body_end + 1;
			return true;
		}

	}// namespace

	TranslationUnit Parser::parse(std::string_view source, std::string_view source_name)
	{
		TranslationUnit unit;
		unit.source_name = std::move(source_name);

		Tokenizer tokenizer;
		auto tokens = tokenizer.tokenize(source);
		parse_range(unit, tokens, source, 0, tokens.size() - 1, {});

		return unit;
	}

}// namespace Reflector
