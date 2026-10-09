#include <algorithm>
#include <cctype>
#include <iostream>
#include <limits>
#include <memory>
#include <model.hpp>
#include <optional>
#include <parser.hpp>
#include <set>
#include <string>
#include <tokenizer.hpp>
#include <unordered_map>
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

		struct Annotation {
			std::string name;
			std::string arguments;
			std::vector<Metadata> metadata;
			std::size_t line   = 0;
			std::size_t column = 0;
		};

		struct DeclarationPrefix {
			std::string attributes;
			std::string engine_macros;
		};

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

		template<typename... Args>
		std::string concatenate(Args&&... args)
		{
			std::string result;
			const std::size_t size = (std::string_view(args).size() + ...);

			result.reserve(size);
			(result.append(std::string_view(args)), ...);

			return result;
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
			static const std::set<std::string_view> prefixes = {
			        "static",  "virtual",  "inline", "constexpr",    "consteval",    "explicit",   "friend",
			        "mutable", "volatile", "extern", "thread_local", "FORCE_INLINE", "DEPRECATED",
			};
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
			return index > 0 && tokens[index - 1].type == TokenType::Identifier && tokens[index - 1].value != "operator" &&
			       is_symbol(tokens[index], "<");
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

		bool parse_argument(const std::vector<Token>& tokens, Function::Argument& argument, std::string_view source,
		                    std::size_t begin, std::size_t end);

		bool parse_type_info(std::string_view raw_type, TypeInfo& info)
		{
			info                = {};
			const auto spelling = trim(raw_type);
			auto tokens         = tokenize(spelling);
			std::erase_if(tokens, is_trivia);
			std::size_t begin = 0;
			std::size_t end   = tokens.size() - 1;
			if (begin == end)
				return true;

			std::size_t type_end      = end;
			std::size_t pointer_depth = 0;
			bool function_declarator  = false;
			auto add_pointer          = [&] {
				if (++pointer_depth > 1)
					return false;
				info.flags |= TypeInfo::Pointer;
				return true;
			};
			for (std::size_t i = begin; i < end; ++i)
			{
				const auto& token = tokens[i];
				if (is_template_argument_list(tokens, i))
				{
					const auto close = matching_token(tokens, i);
					if (close > i)
					{
						i = close;
						continue;
					}
				}
				if (token.value == "(")
				{
					const auto close = matching_token(tokens, i);
					if (close == i)
						return false;
					if (i > 0 && (tokens[i - 1].value == "decltype" || tokens[i - 1].value == "typeof"))
					{
						i = close;
						continue;
					}
					// Function signatures still use their C++ spelling, but their pointer
					// depth and parameter types are validated under the same restrictions.
					bool declarator_group = false;
					for (auto j = i + 1; j < close; ++j)
						declarator_group |= tokens[j].value == "*" || tokens[j].value == "&" || tokens[j].value == "&&";
					if (declarator_group && close + 1 < end && (tokens[close + 1].value == "(" || tokens[close + 1].value == "["))
					{
						for (auto j = i + 1; j < close; ++j)
							if (tokens[j].value == "*" && !add_pointer())
								return false;
						function_declarator = true;
					}
					else
					{
						for (const auto& [first, last] : split_token_ranges(tokens, i + 1, close, ","))
						{
							Function::Argument argument;
							if (!parse_argument(tokens, argument, spelling, first, last))
								return false;
						}
						function_declarator = true;
					}
					i = close;
					continue;
				}
				if (token.value == "[")
				{
					if (!add_pointer())
						return false;
					type_end         = std::min(type_end, i);
					const auto close = matching_token(tokens, i);
					if (close == i)
						return false;
					i = close;
					continue;
				}
				if (token.value == "*")
				{
					if (!add_pointer())
						return false;
					type_end = std::min(type_end, i);
				}
				else if (token.value == "&" || token.value == "&&")
				{
					info.flags |= token.value == "&" ? TypeInfo::Reference : TypeInfo::RValueRef;
					type_end = std::min(type_end, i);
				}
				else if (pointer_depth && (token.value == "const" || token.value == "volatile"))
					info.flags |= token.value == "const" ? TypeInfo::PointerConst : TypeInfo::PointerVolatile;
			}
			if (function_declarator)
			{
				info.name = spelling;
				return true;
			}
			end = type_end;
			while (begin < end && (tokens[begin].value == "const" || tokens[begin].value == "volatile"))
				info.flags |= tokens[begin++].value == "const" ? TypeInfo::Const : TypeInfo::Volatile;
			while (begin < end && (tokens[end - 1].value == "const" || tokens[end - 1].value == "volatile"))
				info.flags |= tokens[--end].value == "const" ? TypeInfo::Const : TypeInfo::Volatile;
			if (begin == end)
				return true;

			std::size_t open = end;
			for (auto i = begin; i < end; ++i)
			{
				if (is_template_argument_list(tokens, i))
				{
					open = i;
					break;
				}
			}
			const auto close = open < end ? matching_token(tokens, open) : end;
			if (open == end || close != end - 1)
			{
				// This also preserves dependent nested names such as Outer<T>::Inner.
				// Validate both components so a nested name cannot hide forbidden pointers.
				if (open < end && close > open && close < end)
				{
					TypeInfo component;
					if (!parse_type_info(raw_between(spelling, tokens[begin], tokens[close + 1]), component) ||
					    !parse_type_info(raw_between(spelling, tokens[close + 1], tokens[end]), component))
						return false;
				}
				info.name = raw_between(spelling, tokens[begin], tokens[end]);
				return true;
			}
			info.name = raw_between(spelling, tokens[begin], tokens[open]);
			info.flags |= TypeInfo::Template;
			for (const auto& [first, last] : split_token_ranges(tokens, open + 1, close, ","))
			{
				TypeInfo::TemplateArgument argument;
				auto argument_end = last;
				if (argument_end >= first + 3 && tokens[argument_end - 1].value == "." && tokens[argument_end - 2].value == "." &&
				    tokens[argument_end - 3].value == ".")
				{
					argument.flags |= TypeInfo::TemplateArgument::PackExpansion;
					argument_end -= 3;
				}
				bool value = false;
				for (auto i = first; i < argument_end; ++i)
				{
					if (is_template_argument_list(tokens, i))
					{
						const auto nested_close = matching_token(tokens, i);
						if (nested_close > i)
						{
							i = nested_close;
							continue;
						}
					}
					if (tokens[i].value == "[")
					{
						const auto close = matching_token(tokens, i);
						if (close > i)
						{
							i = close;
							continue;
						}
					}
					value |= tokens[i].type == TokenType::Number || tokens[i].type == TokenType::String ||
					         tokens[i].type == TokenType::Character || tokens[i].value == "true" || tokens[i].value == "false" ||
					         tokens[i].value == "nullptr" || tokens[i].value == "+" || tokens[i].value == "-" ||
					         tokens[i].value == "|" || tokens[i].value == "sizeof" || tokens[i].value == "alignof";
				}
				const auto text = raw_between(spelling, tokens[first], tokens[argument_end]);
				if (value)
				{
					argument.flags =
					        (argument.flags & TypeInfo::TemplateArgument::PackExpansion) | TypeInfo::TemplateArgument::Value;
					argument.value = text;
				}
				else
				{
					if (!parse_type_info(text, argument.type))
						return false;
				}
				info.templates.push_back(std::move(argument));
			}
			return true;
		}


		std::vector<Metadata> parse_annotation_metadata(std::string_view arguments)
		{
			auto tokens = tokenize(arguments);
			std::vector<Metadata> output;

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

				Metadata argument;
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

		std::size_t find_property_name(const std::vector<Token>& tokens, std::size_t begin, std::size_t end)
		{
			std::size_t best = end;
			for (std::size_t i = begin; i < end;)
			{
				if (is_symbol(tokens[i], "("))
				{
					const auto close = matching_token(tokens, i);
					if (close + 1 < end && (is_symbol(tokens[close + 1], "(") || is_symbol(tokens[close + 1], "[")))
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

		bool is_builtin(std::string_view name)
		{
			static const std::set<std::string_view> names = {"void",     "bool",     "char",  "char8_t", "char16_t",
			                                                 "char32_t", "wchar_t",  "short", "int",     "long",
			                                                 "signed",   "unsigned", "float", "double",  "auto"};
			return names.contains(name);
		}

		bool parse_argument(const std::vector<Token>& tokens, Function::Argument& argument, std::string_view source,
		                    std::size_t begin, std::size_t end)
		{
			argument = {};
			while (begin < end && tokens[begin].value.starts_with("trinex_"))
			{
				Cursor cursor{source, &tokens, begin};
				Annotation ignored;
				if (!parse_annotation(cursor, ignored))
					break;
				begin = cursor.index;
			}
			const auto equals = find_top_level_equal(tokens, begin, end);
			if (equals != end)
			{
				argument.value = raw_between(source, tokens[equals + 1], tokens[end]);
				end            = equals;
			}
			auto name = find_property_name(tokens, begin, end);
			if (name != end &&
			    (is_builtin(tokens[name].value) || name == begin || (name > begin && tokens[name - 1].value == "::") ||
			     (name + 1 < end && (tokens[name + 1].value == "::" || tokens[name + 1].value == "<"))))
				name = end;
			if (name != end)
			{
				const auto spelling =
				        strip_type_prefixes(declaration_without_token(source, tokens[begin], tokens[end], tokens[name]));
				TypeInfo type;
				if (!parse_type_info(spelling, type))
					return false;
				if (!type.name.empty())
				{
					argument.name = tokens[name].value;
					argument.type = std::move(type);
					return true;
				}
			}
			return parse_type_info(strip_type_prefixes(raw_between(source, tokens[begin], tokens[end])), argument.type);
		}

		// Evaluate the integer constant expressions supported by the model. Unknown
		// symbols/expressions are diagnosed rather than silently assigned zero.
		class EnumExpression
		{
			const std::vector<Token>& tokens;
			const std::unordered_map<std::string, std::int64_t>& values;
			std::size_t index;
			std::size_t end;

			std::optional<std::int64_t> primary()
			{
				if (index >= end)
					return {};
				const auto token = tokens[index++];
				if (token.value == "(")
				{
					auto result = expression(0);
					if (index >= end || tokens[index++].value != ")")
						return {};
					return result;
				}
				if (token.value == "+" || token.value == "-" || token.value == "~" || token.value == "!")
				{
					auto value = primary();
					if (!value)
						return {};
					if (token.value == "-")
					{
						if (*value == std::numeric_limits<std::int64_t>::min())
							return {};
						return -*value;
					}
					if (token.value == "~")
						return ~*value;
					if (token.value == "!")
						return !*value;
					return value;
				}
				if (token.value == "true")
					return 1;
				if (token.value == "false")
					return 0;
				if (token.type == TokenType::Identifier)
				{
					auto name = std::string(token.value);

					while (index + 1 < end && tokens[index].value == "::")
					{
						name += "::";
						name += tokens[index + 1].value;
						index += 2;
					}
					auto found = values.find(name);
					if (found != values.end())
						return found->second;
					return {};
				}
				if (token.type == TokenType::Character && token.value.size() == 3)
					return static_cast<unsigned char>(token.value[1]);
				if (token.type != TokenType::Number)
					return {};
				try
				{
					std::size_t consumed = 0;
					const bool binary    = token.value.starts_with("0b") || token.value.starts_with("0B");
					const auto text      = std::string(binary ? token.value.substr(2) : token.value);
					const auto value     = std::stoll(text, &consumed, binary ? 2 : 0);
					if (text.substr(consumed).find_first_not_of("uUlL") != std::string::npos)
						return {};
					return value;
				}
				catch (const std::exception&)
				{
					return {};
				}
			}

			std::optional<std::int64_t> expression(int minimum)
			{
				auto left = primary();
				while (left && index < end)
				{
					auto op            = std::string(tokens[index].value);
					std::size_t length = 1;
					if ((op == "<" || op == ">") && index + 1 < end && tokens[index + 1].value == op)
					{
						op += op;
						length = 2;
					}
					int precedence = op == "|"                               ? 1
					                 : op == "^"                             ? 2
					                 : op == "&"                             ? 3
					                 : (op == "<<" || op == ">>")            ? 4
					                 : (op == "+" || op == "-")              ? 5
					                 : (op == "*" || op == "/" || op == "%") ? 6
					                                                         : -1;
					if (precedence < minimum)
						break;
					index += length;
					auto right = expression(precedence + 1);
					if (!right)
						return {};
					std::int64_t result = 0;
					if (op == "+")
					{
						if ((*right > 0 && *left > std::numeric_limits<std::int64_t>::max() - *right) ||
						    (*right < 0 && *left < std::numeric_limits<std::int64_t>::min() - *right))
							return {};
						result = *left + *right;
					}
					else if (op == "-")
					{
						if ((*right < 0 && *left > std::numeric_limits<std::int64_t>::max() + *right) ||
						    (*right > 0 && *left < std::numeric_limits<std::int64_t>::min() + *right))
							return {};
						result = *left - *right;
					}
					else if (op == "*")
					{
						const auto min = std::numeric_limits<std::int64_t>::min();
						const auto max = std::numeric_limits<std::int64_t>::max();
						if (*left > 0 && ((*right > 0 && *left > max / *right) || (*right < 0 && *right < min / *left)))
							return {};
						if (*left < 0 && ((*right > 0 && *left < min / *right) || (*right < 0 && *left < max / *right)))
							return {};
						result = *left * *right;
					}
					else if (op == "/" || op == "%")
					{
						if (*right == 0 || (*left == std::numeric_limits<std::int64_t>::min() && *right == -1))
							return {};
						result = op == "/" ? *left / *right : *left % *right;
					}
					else if (op == "|")
						result = *left | *right;
					else if (op == "^")
						result = *left ^ *right;
					else if (op == "&")
						result = *left & *right;
					else
					{
						if (*right < 0 || *right >= 63 || *left < 0)
							return {};
						if (op == "<<" && *left > (std::numeric_limits<std::int64_t>::max() >> *right))
							return {};
						result = op == "<<" ? *left << *right : *left >> *right;
					}
					left = result;
				}
				return left;
			}

		public:
			EnumExpression(const std::vector<Token>& tokens, std::size_t begin, std::size_t end,
			               const std::unordered_map<std::string, std::int64_t>& values)
			    : tokens(tokens), values(values), index(begin), end(end)
			{}

			std::optional<std::int64_t> evaluate()
			{
				auto result = expression(0);
				return index == end ? result : std::nullopt;
			}
		};

		class ModuleParser
		{
		private:
			std::string_view source;
			std::vector<Token> tokens;

			bool failure(const Annotation& annotation, std::string_view message)
			{
				std::cerr << "Error [" << annotation.line << ':' << annotation.column << "]: " << message << '\n';
				return false;
			}

			bool failure(const Token& token, std::string_view message)
			{
				std::cerr << "Error [" << token.line << ':' << token.column << "]: " << message << '\n';
				return false;
			}

			void attach(Scope& scope, Object* object)
			{
				object->owner = &scope;
				scope.objects.push_back(object);
			}

			bool property(Scope& scope, const Annotation& annotation, std::size_t begin, std::size_t end, Access access)
			{
				auto object          = std::make_unique<Property>();
				object->metadata     = annotation.metadata;
				object->access       = access;
				auto declaration_end = find_top_level_equal(tokens, begin, end);
				if (declaration_end < end)
					object->value = raw_between(source, tokens[declaration_end + 1], tokens[end]);
				for (auto i = begin; i < declaration_end;)
				{
					if (tokens[i].value == ":")
					{
						object->flags |= Property::Bitfield;
						declaration_end = i;
						break;
					}
					if (tokens[i].value == "{")
					{
						object->value   = raw_between(source, tokens[i], tokens[declaration_end]);
						declaration_end = i;
						break;
					}
					i = skip_balanced(tokens, i);
				}
				const auto name = find_property_name(tokens, begin, declaration_end);
				if (name == declaration_end || name == begin || is_builtin(tokens[name].value))
				{
					return failure(annotation, "reflected property has no parsed name");
				}
				if (split_token_ranges(tokens, begin, end, ",").size() > 1)
				{
					return failure(annotation, "multiple reflected declarators are not supported");
				}
				object->name = tokens[name].value;
				if (!parse_type_info(strip_type_prefixes(declaration_without_token(source, tokens[begin], tokens[declaration_end],
				                                                                   tokens[name])),
				                     object->type))
					return failure(annotation, "failed to parse property type");
				for (auto i = begin; i < name; ++i)
				{
					if (is_template_argument_list(tokens, i))
					{
						i = matching_token(tokens, i);
						continue;
					}
					if (tokens[i].value == "static")
						object->flags |= Property::Static;
					if (tokens[i].value == "constexpr")
						object->flags |= Property::Constexpr;
					if (tokens[i].value == "mutable")
						object->flags |= Property::Mutable;
					if (tokens[i].value == "inline")
						object->flags |= Property::Inline;
				}
				if (object->type.name.empty())
					return failure(annotation, "reflected property has no parsed type");
				attach(scope, object.release());
				return true;
			}

			bool function(Scope& scope, const Annotation& annotation, std::size_t begin, std::size_t end, Access access,
			              bool is_template, const DeclarationPrefix& prefix)
			{
				auto object      = std::make_unique<Function>();
				object->metadata = annotation.metadata;
				object->access   = access;
				if (is_template)
				{
					object->flags |= Function::Template;
					return failure(annotation, "reflected template functions are not supported");
				}
				auto open = find_parameter_list_open(tokens, begin, end);
				if (open < end && open > begin && tokens[open - 1].value == "operator" &&
				    matching_token(tokens, open) == open + 1 && open + 2 < end && tokens[open + 2].value == "(")
					open += 2;
				const auto close = open < end ? matching_token(tokens, open) : end;
				if (open == end || open == begin || close == open || close >= end)
				{
					return failure(annotation, "failed to parse annotated function");
				}
				auto name = open - 1;
				for (auto i = begin; i < open; ++i)
				{
					if (tokens[i].value == "operator")
					{
						name = i;
						object->flags |= Function::Operator;
						break;
					}
				}
				if (name > begin && tokens[name - 1].value == "~")
				{
					--name;
					object->flags |= Function::Destructor;
				}
				object->name = raw_between(source, tokens[name], tokens[open]);
				if (!parse_type_info(strip_type_prefixes(raw_between(source, tokens[begin], tokens[name])), object->type))
					return failure(annotation, "failed to parse return type");
				if (object->type.name.empty() && !(object->flags & (Function::Operator | Function::Destructor)))
				{
					if (dynamic_cast<Struct*>(&scope) && object->name == scope.name)
						object->flags |= Function::Constructor;
					else
						return failure(annotation, "reflected function has no parsed return type");
				}
				if ((object->flags & Function::Operator) && object->type.name.empty() && name + 1 < open &&
				    tokens[name + 1].type == TokenType::Identifier)
				{
					if (!parse_type_info(raw_between(source, tokens[name + 1], tokens[open]), object->type))
						return failure(annotation, "failed to parse conversion type");
				}
				for (auto i = begin; i < name; ++i)
				{
					if (is_template_argument_list(tokens, i))
					{
						i = matching_token(tokens, i);
						continue;
					}
					if (tokens[i].value == "static")
						object->flags |= Function::Static;
					if (tokens[i].value == "virtual")
						object->flags |= Function::Virtual;
					if (tokens[i].value == "constexpr")
						object->flags |= Function::Constexpr;
					if (tokens[i].value == "inline")
						object->flags |= Function::Inline;
				}
				if (contains_word(prefix.engine_macros, "FORCE_INLINE") || contains_word(prefix.engine_macros, "FORCEINLINE"))
					object->flags |= Function::Inline;
				for (auto i = close + 1; i < end; ++i)
				{
					const auto& text = tokens[i].value;
					if (text == "->")
					{
						auto type_end = i + 1;
						while (type_end < end && tokens[type_end].value != "override" && tokens[type_end].value != "final" &&
						       tokens[type_end].value != "=")
							++type_end;
						if (!parse_type_info(raw_between(source, tokens[i + 1], tokens[type_end]), object->type))
							return failure(annotation, "failed to parse trailing return type");
						i = type_end - 1;
						continue;
					}
					if (text == ":")
						break;// Constructor initializer list.
					if (text == "const")
						object->flags |= Function::Const;
					if (text == "volatile")
						object->flags |= Function::Volatile;
					if (text == "&")
						object->flags |= Function::Reference;
					if (text == "&&")
						object->flags |= Function::RValueRef;
					if (text == "override")
						object->flags |= Function::Override | Function::Virtual;
					if (text == "final")
						object->flags |= Function::Final;
					if (text == "=" && i + 1 < end && tokens[i + 1].value == "0")
						object->flags |= Function::PureVirtual | Function::Virtual;
					if (text == "noexcept")
					{
						if (i + 1 < end && tokens[i + 1].value == "(")
						{
							const auto last  = matching_token(tokens, i + 1);
							const auto value = EnumExpression(tokens, i + 2, last, {}).evaluate();
							if (!value)
								return failure(annotation, "unable to evaluate noexcept expression");
							else if (*value)
								object->flags |= Function::Noexcept;
							i = last;
						}
						else
							object->flags |= Function::Noexcept;
					}
				}
				if (close != open + 2 || tokens[open + 1].value != "void")
				{
					for (const auto& [first, last] : split_token_ranges(tokens, open + 1, close, ","))
					{
						if (last == first + 3 && tokens[first].value == "." && tokens[first + 1].value == "." &&
						    tokens[first + 2].value == ".")
							object->flags |= Function::Variadic;
						else
						{
							Function::Argument argument;
							if (!parse_argument(tokens, argument, source, first, last))
								return failure(annotation, "failed to parse function argument");
							object->args.push_back(std::move(argument));
						}
					}
				}
				attach(scope, object.release());
				return true;
			}

			bool enumeration(Enum& object, const Annotation& annotation, std::size_t begin, std::size_t end)
			{
				std::unordered_map<std::string, std::int64_t> values;
				std::optional<std::int64_t> next = 0;
				for (const auto& [first, last] : split_token_ranges(tokens, begin, end, ","))
				{
					Cursor cursor{source, &tokens, first};
					Annotation item_annotation;
					if (tokens[first].value.starts_with("trinex_"))
						parse_annotation(cursor, item_annotation);
					const auto name = cursor.index;
					if (name >= last || !is_identifier(tokens[name]))
					{
						return failure(annotation, "failed to parse enum value");
					}
					const auto equals = find_top_level_equal(tokens, name + 1, last);
					auto value        = equals < last ? EnumExpression(tokens, equals + 1, last, values).evaluate() : next;
					if (!value)
					{
						return failure(annotation, concatenate("unable to evaluate enum value: ", tokens[name].value));
					}
					object.values.push_back({std::string(tokens[name].value), *value, std::move(item_annotation.metadata)});
					values[std::string(tokens[name].value)]                    = *value;
					values[concatenate(object.name, "::", tokens[name].value)] = *value;
					next = *value == std::numeric_limits<std::int64_t>::max() ? std::nullopt : std::optional(*value + 1);
				}
				return true;
			}

			bool record(Scope& scope, Cursor& cursor, const Annotation& annotation, std::size_t end, Access access,
			            bool is_template)
			{
				const auto kind = current(cursor).value;
				if (kind != "class" && kind != "struct" && kind != "enum")
				{
					return failure(annotation, "failed to parse annotated type");
				}
				++cursor.index;
				if (kind == "enum" && (current(cursor).value == "class" || current(cursor).value == "struct"))
					++cursor.index;
				consume_declaration_prefix(cursor);
				if (!is_identifier(current(cursor)))
				{
					return failure(annotation, "reflected type has no parsed name");
				}
				std::unique_ptr<Object> object;

				if (kind == "class")
					object = std::make_unique<Class>();
				else if (kind == "struct")
					object = std::make_unique<Struct>();
				else
					object = std::make_unique<Enum>();

				object->name     = current(cursor).value;
				object->access   = access;
				object->metadata = annotation.metadata;
				++cursor.index;
				auto* structure = dynamic_cast<Struct*>(object.get());
				if (is_template)
					return failure(annotation, "reflected template classes are not supported: " + object->name);
				if (current(cursor).value == "final" && structure)
				{
					structure->flags |= Struct::Final;
					++cursor.index;
				}
				const auto bases_begin = current(cursor).value == ":" ? ++cursor.index : end;
				while (cursor.index < end && current(cursor).value != "{" && current(cursor).value != ";") ++cursor.index;
				if (cursor.index >= end)
				{
					return failure(annotation, "failed to parse annotated type body");
				}
				if (structure && bases_begin < cursor.index)
				{
					for (const auto& [first, last] : split_token_ranges(tokens, bases_begin, cursor.index, ","))
					{
						Struct::Base base;
						base.access = kind == "class" ? Access::Private : Access::Public;
						auto begin  = first;
						while (begin < last)
						{
							const auto& word = tokens[begin].value;
							if (word == "virtual")
								base.flags |= Struct::Base::Virtual;
							else if (word == "public")
								base.access = Access::Public;
							else if (word == "private")
								base.access = Access::Private;
							else if (word == "protected")
								base.access = Access::Protected;
							else
								break;
							++begin;
						}
						if (!parse_type_info(raw_between(source, tokens[begin], tokens[last]), base.type))
							return failure(annotation, "failed to parse base type");
						structure->bases.push_back(std::move(base));
					}
				}
				object->owner = &scope;
				if (current(cursor).value == "{")
				{
					const auto close = matching_token(tokens, cursor.index);
					if (close == cursor.index || close > end)
					{
						return failure(annotation, "unclosed annotated type body");
					}
					if (structure)
					{
						if (!range(*structure, cursor.index + 1, close, kind == "class" ? Access::Private : Access::Public))
							return false;
					}
					else if (!enumeration(static_cast<Enum&>(*object), annotation, cursor.index + 1, close))
						return false;
					cursor.index = close + 1;
				}
				if (current(cursor).value == ";")
					++cursor.index;
				attach(scope, object.release());
				return true;
			}

			bool namespace_scope(Scope& scope, Cursor& cursor, std::size_t end)
			{
				auto index = cursor.index;
				if (tokens[index].value == "inline")
					++index;
				const auto start = index++;
				std::vector<std::string_view> parts;
				while (index < end && tokens[index].value != "{" && tokens[index].value != "=" && tokens[index].value != ";")
				{
					if (is_identifier(tokens[index]))
						parts.push_back(tokens[index].value);
					++index;
				}
				if (index == end || tokens[index].value != "{")
				{
					while (index < end && tokens[index].value != ";") ++index;
					cursor.index = index < end ? index + 1 : end;
					return true;
				}
				const auto close = matching_token(tokens, index);
				if (close == index || close > end)
				{
					return failure(tokens[start], "unclosed namespace body");
				}
				if (parts.empty())
					parts.emplace_back();// Anonymous namespace is still a distinct scope.
				Scope* owner = &scope;
				for (const auto& part : parts)
				{
					Namespace* nested = nullptr;
					for (const auto& child : owner->objects)
						if (child->is_a(ObjectKind::Namespace) && child->name == part)
							nested = static_cast<Namespace*>(child);
					if (!nested)
					{
						auto created  = new Namespace();
						created->name = part;
						nested        = created;
						attach(*owner, created);
					}
					owner = nested;
				}
				if (!range(*owner, index + 1, close, Access::Global))
					return false;
				cursor.index = close + 1;
				return true;
			}

			bool range(Scope& scope, std::size_t begin, std::size_t end, Access access)
			{
				Cursor cursor{source, &tokens, begin};
				while (cursor.index < end)
				{
					const auto start = cursor.index;
					if (cursor.index + 1 < end && tokens[cursor.index + 1].value == ":")
					{
						const auto& word = current(cursor).value;
						if (word == "public" || word == "private" || word == "protected")
						{
							access = word == "public" ? Access::Public : word == "private" ? Access::Private : Access::Protected;
							cursor.index += 2;
							continue;
						}
					}
					if (current(cursor).value == "namespace" || (current(cursor).value == "inline" && cursor.index + 1 < end &&
					                                             tokens[cursor.index + 1].value == "namespace"))
					{
						if (!namespace_scope(scope, cursor, end))
							return false;
						continue;
					}
					auto template_prefix = consume_template_prefix(cursor);
					auto prefix          = consume_declaration_prefix(cursor);
					Annotation annotation;
					if (!parse_annotation(cursor, annotation))
					{
						if (tokens[start].value == "trinex_class" || tokens[start].value == "trinex_struct" ||
						    tokens[start].value == "trinex_enum" || tokens[start].value == "trinex_function" ||
						    tokens[start].value == "trinex_property")
						{
							return failure(tokens[start], "malformed reflection annotation");
						}

						// Skip unreflected bodies rather than importing local declarations into the parent scope.
						if (current(cursor).value == "{")
						{
							const auto close = matching_token(tokens, cursor.index);
							cursor.index     = close > cursor.index ? close + 1 : end;
						}
						else
							cursor.index = std::max(cursor.index, start + 1);
						continue;
					}
					if (template_prefix.empty())
						template_prefix = consume_template_prefix(cursor);
					const auto after     = consume_declaration_prefix(cursor);
					prefix.engine_macros = append_raw(prefix.engine_macros, after.engine_macros);
					if (annotation.name == "trinex_class" || annotation.name == "trinex_struct" ||
					    annotation.name == "trinex_enum")
					{
						if (!record(scope, cursor, annotation, end, access, !template_prefix.empty()))
							return false;
						cursor.index = std::max(cursor.index, start + 1);
						continue;
					}
					if (annotation.name != "trinex_property" && annotation.name != "trinex_function")
						continue;
					const auto declaration_begin = cursor.index;
					auto declaration_end         = std::min(find_statement_end(tokens, declaration_begin), end);
					if (declaration_end == end)
					{
						return failure(annotation, "unterminated reflected declaration");
					}
					cursor.index = declaration_end + 1;
					if (tokens[declaration_end].value == "{")
					{
						const auto close = matching_token(tokens, declaration_end);
						if (close == declaration_end || close > end)
						{
							return failure(annotation, "unclosed reflected declaration body");
						}
						cursor.index = close + 1;
						if (annotation.name == "trinex_property")
							declaration_end = close + 1;
					}
					if (annotation.name == "trinex_property")
					{
						if (!property(scope, annotation, declaration_begin, declaration_end, access))
							return false;
					}
					else if (!function(scope, annotation, declaration_begin, declaration_end, access, !template_prefix.empty(),
					                   prefix))
						return false;
				}
				return true;
			}

		public:
			ModuleParser(std::string_view source) : source(source), tokens(tokenize(source)) { std::erase_if(tokens, is_trivia); }
			bool parse(Module& module) { return range(module, 0, tokens.size() - 1, Access::Global); }
		};
	}// namespace

	Module* parse(std::string_view source, std::string_view source_name)
	{
		auto module  = new Module();
		module->name = source_name;

		if (!ModuleParser(source).parse(*module))
		{
			delete module;
			return nullptr;
		}

		return module;
	}
}// namespace Reflector
